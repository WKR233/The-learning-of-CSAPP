/*
 * mm.c
 *
 * 王柯然 2100017727
 * This allocator used the technique of segregated fits in 
 * Segregated Free Lists to mark the free blocks
 * 
 * the free block is made up of 6 parts
 * the first part is header, 4 bytes long
 * followed by 2 pointers, each of which is 4 bytes long, points to the 
 * previous same type block and next same type block
 * the 4th part is used for data, 2nd, 3rd, 4th parts compose payload
 * the 5th part is padding, used for alignment
 * the 6th part is footer, footer==header
 * 
 * the used block is made up of 3 parts
 * the 1st part is header, same as which in free block
 * the 2nd part is payload, the 3rd part is footer, same as header
 * 
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUG
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* how many classes we want */
#define CLASS 18

/* Basic constants and macros */
#define WSIZE       4       /* Word and header/footer size (bytes) */ 
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE  (1<<12)  /* Extend heap by this amount (bytes) */  

#define MAX(x, y) ((x) > (y)? (x) : (y))  

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc)) 

/* Translate a relative addr to a abs addr */
#define ADDR(bp)     ((unsigned int *)((bp)+heap_head))

/* Translate a abs addr to a relative addr */
#define OFST(ptr)    ((unsigned int)((char *)(ptr)-heap_head))

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(ADDR(p)))            
#define PUT(p, val)  (*(unsigned int *)(ADDR(p)) = (unsigned int)(val))    

/* Read the size and allocated fields from address p */
#define GET_SIZE(p)  (GET(p) & ~0x7)                   
#define GET_ALLOC(p) (GET(p) & 0x1)                    

/* Given block ptr bp, compute address of its header and footer */
#define HDRP(bp)       ((bp) - WSIZE)                      
#define FTRP(bp)       ((bp)+ GET_SIZE(HDRP(bp)) - DSIZE)

/* find the block that next to bp */
#define AFTR_BLKP(bp)       ((bp) + GET_SIZE(HDRP(bp)))
#define BEFR_BLKP(bp)       ((bp) - GET_SIZE(HDRP(bp)-WSIZE))

/* the pointer of prev and next */
#define NEXT(bp)       ((bp) + WSIZE)
#define PREV(bp)       ((bp))

/* Given block ptr bp, if bp is allocated, compute the next allocated block
 * else, compute the next same class free block */

#define NEXT_BLKP(bp)  ((unsigned int)(*ADDR(NEXT(bp)))) 
#define PREV_BLKP(bp)  ((unsigned int)(*ADDR(PREV(bp)))) 

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(ADDR(p)) + (ALIGNMENT-1)) & ~0x7)

/* Global variables */
static char *heap_listp=0; /* Pointer to the first block */
static char *heap_head=0; /* Pointer to the heap head */

/* declaration of functions */
int mm_init(void);
static unsigned int find_type(int words);
static void list_insert(unsigned int bp, unsigned int classtype);
static void *coalesce(unsigned int bp);
static void *extend_heap(size_t words);
static void place(unsigned int bp,size_t size);
static unsigned int find_fit(size_t size);
void list_delete(unsigned int prev,unsigned int next,unsigned int bp);
void *mm_malloc (size_t size);
void mm_free(void *ptr);
void *mm_realloc(void *oldptr, size_t size);
void *mm_calloc (size_t nmemb, size_t size);
static int in_heap(const unsigned int p);
static int aligned(const unsigned int p);
void mm_checkheap(int lineno);

/*
 * Initialize: return -1 on error, 0 on success.
 * create CLASS classes, class i: {2^{i+1}~2^{i+2}-1} words
 * class 1: {4~7}, class 2: {8~15}, ..., class CLASS: {2^{CLASS+1}~infinity}
 * set CLASS offset, each offset requires 4 bytes,
 */
int mm_init(void)
{
    /* Create the initial empty heap */
    if((heap_listp=mem_sbrk((4+CLASS)*WSIZE))==(void *)-1) /* failed to create */
        return -1;
    heap_head=heap_listp;
    PUT(0,0); /* initialize the heap_head */
    for(int classno=1;classno<=CLASS;classno++) /* initialize the class head */
        PUT(classno*WSIZE,0);
    PUT(((1+CLASS)*WSIZE),PACK(DSIZE,1));
    PUT(((2+CLASS)*WSIZE),PACK(DSIZE,1));
    PUT(((3+CLASS)*WSIZE),PACK(0,1));
    heap_listp+=(2+CLASS)*WSIZE;

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if(extend_heap(CHUNKSIZE/WSIZE)==NULL)
        return -1;
    mm_checkheap(__LINE__);
    return 0;
}

/*
 * find_type: return the class type corresponding to words
 */
static unsigned int find_type(int words)
{
    int classtype;
    int tmpwords;
    tmpwords=words;
    for(classtype=-2;tmpwords!=0;classtype++)
        tmpwords=tmpwords>>1;
    return classtype>CLASS?CLASS:classtype; /* return max(classtype,CLASS) */
}

/*
 * list_insert: insert the given bp to classtype list, but DO NOT set HDR/FTR
*/
static void list_insert(unsigned int bp, unsigned int classtype)
{
    unsigned int num=GET(classtype*WSIZE);
    if(num==0) /* it's the first block linked to classtype */
    {
        GET(classtype*WSIZE)=bp;
        GET(PREV(bp))=classtype*WSIZE;
        GET(NEXT(bp))=0;
    }
    else /* change 4 pointers */
    {
        GET(PREV(num))=bp;
        GET(NEXT(bp))=num;
        GET(PREV(bp))=classtype*WSIZE;
        GET(classtype*WSIZE)=bp;
    }
}

/*
 * coalesce: only called at two cases:
 * 1. extend_heap, merge new free block with old epilogue
 * 2. free a block
 * it is used for Coalescing with Boundary Tags, with seperated free lists, 
 * we also need to compute the new size and change the lists
 */
static void *coalesce(unsigned int bp)
{
    /* two uint, equals 1 if the prev/next block is allocated*/
    size_t aftr_alloc=GET_ALLOC(HDRP(AFTR_BLKP(bp))); 
    size_t befr_alloc=GET_ALLOC(HDRP(BEFR_BLKP(bp)));
    size_t size=GET_SIZE(HDRP(bp));
    unsigned int new_classtype=1;
    unsigned int befr_bp,aftr_bp,old_bp;
    unsigned int prev_old_bp, next_old_bp, prev_aftr_bp, next_aftr_bp,
                 prev_befr_bp, next_befr_bp;

    /*
     * Set the pointers of each block
     */
    befr_bp=BEFR_BLKP(bp);
    aftr_bp=AFTR_BLKP(bp);
    old_bp=bp;
    prev_old_bp=PREV_BLKP(bp);
    next_old_bp=NEXT_BLKP(bp);

    if(befr_alloc&&aftr_alloc) /* no need to coalesce */
    {
        return ADDR(bp);
    }
    /*
     * able to coalesce with aftr block
     * need to change the seperated free list
    */
    if(befr_alloc&&!aftr_alloc) 
    {
        size+=GET_SIZE(HDRP(AFTR_BLKP(bp)));
        new_classtype=find_type(size/WSIZE);

        /* Delete the old_bp in lists */
        list_delete(prev_old_bp,next_old_bp,old_bp);
        GET(FTRP(old_bp))=0;

        /* Delete the aftr_bp in lists */
        prev_aftr_bp=PREV_BLKP(aftr_bp);
        next_aftr_bp=NEXT_BLKP(aftr_bp);
        list_delete(prev_aftr_bp,next_aftr_bp,aftr_bp);
        GET(HDRP(aftr_bp))=0;

        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }

    /*
     * able to coalesce with befr block
     * need to change the seperated free list
     */
    else if(!befr_alloc&&aftr_alloc) /* able to coalesce with befr block*/
    {
        size+=GET_SIZE(HDRP(BEFR_BLKP(bp)));
        new_classtype=find_type(size/WSIZE);
        bp=BEFR_BLKP(bp);


        /* Delete the old_bp in lists */
        list_delete(prev_old_bp,next_old_bp,old_bp);
        GET(HDRP(old_bp))=0;

        /* Delete the befr_bp in lists */
        prev_befr_bp=PREV_BLKP(befr_bp);
        next_befr_bp=NEXT_BLKP(befr_bp);
        list_delete(prev_befr_bp,next_befr_bp,befr_bp);
        GET(FTRP(befr_bp))=0;

        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }

    /*
     * able to coalesce with both sides
     */
    else if(!befr_alloc&&!aftr_alloc)
    {
        size+=GET_SIZE(HDRP(BEFR_BLKP(bp)))+GET_SIZE(HDRP(AFTR_BLKP(bp)));
        new_classtype=find_type(size/WSIZE);
        bp=BEFR_BLKP(bp);

        /* Delete the old_bp in lists */
        list_delete(prev_old_bp,next_old_bp,old_bp);
        GET(HDRP(old_bp))=0;
        GET(FTRP(old_bp))=0;

        /* Delete the aftr_bp in lists */
        prev_aftr_bp=PREV_BLKP(aftr_bp);
        next_aftr_bp=NEXT_BLKP(aftr_bp);
        list_delete(prev_aftr_bp,next_aftr_bp,aftr_bp);
        GET(HDRP(aftr_bp))=0;

        /* Delete the befr_bp in lists */
        prev_befr_bp=PREV_BLKP(befr_bp);
        next_befr_bp=NEXT_BLKP(befr_bp);
        list_delete(prev_befr_bp,next_befr_bp,befr_bp);
        GET(FTRP(befr_bp))=0;

        PUT(HDRP(bp),PACK(size,0));
        PUT(FTRP(bp),PACK(size,0));
    }

    /* insert the new block */
    list_insert(bp,new_classtype);
    return ADDR(bp);
}

/*
 * extend_heap: ask for a new free block to append to the end of now heap
 * its size should be at least words bytes long
 * return the pointer of new heap top if succeed,
 * return NULL if failed
 */
static void *extend_heap(size_t words)
{
    unsigned int bp;
    size_t size;
    int classtype;

    /* Allocate an even number of words to maintain alignment */
    size=(words%2)?(words+1)*WSIZE:words*WSIZE;
    void *tmpp=mem_sbrk(size);
    if(tmpp==(void *)-1) /* the case that failed to extend heap */
        return NULL;
    bp=OFST(tmpp);

    /* Initialize free block header/footer */
    PUT(HDRP(bp),PACK(size,0));
    PUT(FTRP(bp),PACK(size,0));
    PUT((FTRP(bp)+WSIZE),PACK(0,1)); /* regard this as new epilogue*/

    /* find the classtype */
    classtype=find_type(size/WSIZE);
    list_insert(bp,classtype);
    /* Handle the old epilogue block:if we need to merge it with old epilogue*/
    return coalesce(bp);
}

/*
 * place: with given bp and size, we initialize the HDR/FTR with asize
 * then we set the HDR/FTR of the rest free block
 * the bp used to be a pointer of free block, size is larger than 1 byte
 * NOTE THAT THIS FUNCTION HAS NO ERROR CHECK, USER SHOULD CHECK BY THEMSELVES
*/
static void place(unsigned int bp,size_t size)
{
    size_t csize=GET_SIZE(HDRP(bp));
    unsigned int classtype;

    if((csize-size)>=2*DSIZE)
    {
        list_delete(PREV_BLKP(bp),NEXT_BLKP(bp),bp);
        PUT(HDRP(bp),PACK(size,1));
        PUT(FTRP(bp),PACK(size,1));
        bp=AFTR_BLKP(bp); /* place is a function here, bp will not change*/
        classtype=find_type((csize-size)/WSIZE);
        PUT(HDRP(bp),PACK(csize-size,0));
        PUT(FTRP(bp),PACK(csize-size,0));
        list_insert(bp,classtype);
    }
    else
    {
        list_delete(PREV_BLKP(bp),NEXT_BLKP(bp),bp);
        PUT(HDRP(bp),PACK(csize,1));
        PUT(FTRP(bp),PACK(csize,1));
    }
}

/*
 * find_fit: find the first fit free block of which size is bigger than size
 * size is aligned with DSIZE, includes the overhead
 * if failed to find it, return NULL
 */
static unsigned int find_fit(size_t size)
{
    unsigned int classtype=find_type(size/WSIZE);
    unsigned int i;

    for(i=classtype;i<=CLASS;i++)
    {
        if(GET(i*WSIZE)!=0) /* find a proper space */
        {
            if(i==classtype)
                if(size>GET_SIZE(HDRP(GET(i*WSIZE))))
                    continue;
            break;
        }
    }
    if(i>CLASS) /* failed to find it */
        return 0;
    else
        return GET(i*WSIZE);
}

/*
 * list_delete: delete a block from its list
 */
void list_delete(unsigned int prev,unsigned int next,unsigned int bp)
{
    if(next==0) /* nothing after it */
    {
        if(prev<=CLASS*WSIZE) /* the prev is class */
            GET(prev)=0;
        else
            GET(NEXT(prev))=0;
    }
    else
    {
        if(prev<=CLASS*WSIZE) /* the prev is class */
            GET(prev)=next;
        else
            GET(NEXT(prev))=next;
        GET(PREV(next))=prev;
    }
    GET(PREV(bp))=0;
    GET(NEXT(bp))=0;
}

/*
 * mm_malloc: using the implict free lists to find the first available block
 * if failed to find it, call extend_heap
 * the size should be at least 2*WSIZE
 * ground to the nearest 8 bytes including the overhead bytes
 * if error, return NULL, else return the pointer of the start
 */
void *mm_malloc (size_t size)
{
    size_t asize; /* adjusted size */
    unsigned int bp;

    if(heap_listp==0) /* the heap hasn't been initialized */
        mm_init();

    if(size==0) /* needn't to malloc */
        return NULL;

    asize=DSIZE*((size+(DSIZE)+(DSIZE-1))/DSIZE);
    if((bp=find_fit(asize))!=0) /*successfully find the space*/
    {
        place(bp,asize);
        return ADDR(bp);
    }

    /* failed to find it -> increase*/
    size_t extend_size=MAX(asize,CHUNKSIZE); /* at least increase CHUNKSIZE*/
    void *tmpp=extend_heap(extend_size/WSIZE);
    if(tmpp==NULL)
        return NULL;
    bp=OFST(tmpp);
    place(bp,asize);
    return ADDR(bp);
}

/*
 * mm_free: free a block, add it to list and then coalesce it
 */
void mm_free(void *ptr)
{
    if(ptr==NULL)
        return;
    unsigned int p=OFST(ptr);
    size_t size=GET_SIZE(HDRP(p)); /* get the size of free block */
    if(heap_listp==0)
        mm_init();

    PUT(HDRP(p),PACK(size,0));
    PUT(FTRP(p),PACK(size,0));
    PUT(PREV(p),0);
    PUT(NEXT(p),0);
    if(PREV_BLKP(p)==0) /*it's beginning*/
    {
        if(NEXT_BLKP(p)!=0)
        {
            GET(PREV(NEXT_BLKP(p)))=0;
            GET(NEXT_BLKP(p))=0;
        }
    }
    else /*middle*/
    {
        unsigned int prev,next;
        prev=PREV_BLKP(p);
        next=NEXT_BLKP(p);
        GET(PREV(next))=prev;
        GET(NEXT(prev))=next;
        GET(NEXT_BLKP(p))=0;
        GET(PREV_BLKP(p))=0;
    }

    /* insert it to list and coalesce with other */
    list_insert(p,find_type(size/WSIZE));
    coalesce(p);
}

/*
 * realloc: Change the size of the block by mallocing a new block,
 * copying its data, and freeing the old block.
 */
void *mm_realloc(void *oldptr, size_t size)
{
    size_t oldsize;
    void *newptr;
    int oldp=(int)OFST(oldptr);

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(oldptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(oldptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = GET_SIZE(HDRP(oldp));
    if(size < oldsize) oldsize = size;
    memcpy(newptr, oldptr, oldsize);

    /* Free the old block. */
    mm_free(oldptr);

    return newptr;
}

/*
 * calloc: Allocate the block and set it to zero.
 * This function is not tested by mdriver, but it is
 * needed to run the traces.
 */
void *mm_calloc (size_t nmemb, size_t size)
{
    size_t bytes=nmemb*size;
    void *newptr;

    newptr=mm_malloc(bytes);
    memset(newptr, 0, bytes);

    return newptr;
}


/*
 * Return whether the pointer is in the heap.
 * May be useful for debugging.
 */
static int in_heap(const unsigned int p)
{
    return p <= OFST(mem_heap_hi()) && p >= OFST(mem_heap_lo());
}

/*
 * Return whether the pointer is aligned.
 * May be useful for debugging.
 */
static int aligned(const unsigned int p)
{
    return (size_t)ALIGN(p) == (size_t)p;
}

/*
 * mm_checkheap
 * Checking the heap (implicit list, explicit list, segregated list):
 * – Check epilogue and prologue blocks.
 * – Check each block’s address alignment.
 * – Check heap boundaries.
 * – Check each block’s header and footer: size (minimum size, alignment), 
 * previous/next allocate/free bit consistency, header and footer matching each other.
 * – Check coalescing: no two consecutive free blocks in the heap.
 * Checking the free list (explicit list, segregated list):
 * – All next/previous pointers are consistent (if A’s next pointer points to B, 
 * B’s previous pointer should point to A).
 * – All free list pointers points between mem heap lo() and mem heap high().
 * – Count free blocks by iterating through every block and traversing free 
 * list by pointers and see if they match.
 * – All blocks in each list bucket fall within bucket size range (segregated list).
 */
void mm_checkheap(int lineno)
{
    unsigned int prologue_header=(CLASS+1)*WSIZE;
    unsigned int prologue_footer=(CLASS+2)*WSIZE;
    unsigned int epilogue=mem_heapsize()-WSIZE;
    if(!(GET(prologue_header)==PACK(8,1)))
    {
        printf("prologue_header error at line %d\n",lineno);
        exit(1);
    }
    if(!(GET(prologue_footer)==PACK(8,1)))
    {
        printf("prologue_footer error at line %d\n",lineno);
        exit(2);
    }
    if(!(GET(epilogue)==PACK(0,1)))
    {
        printf("epilogue error at line %d\n",lineno);
        exit(3);
    }

    /* Check each block’s address alignment and heap boundary */
    unsigned int bp;
    bp=(CLASS+4)*WSIZE;
    for(;GET_SIZE(HDRP(bp))>0;bp=AFTR_BLKP(bp));
    if(!(GET(HDRP(bp))==PACK(0,1)))
    {
        printf("aligned error at line %d\n",lineno);
        exit(4);
    }

    /* Check each block */
    for(bp=OFST(heap_listp+2*WSIZE);GET_SIZE(HDRP(bp))>0;bp=AFTR_BLKP(bp))
    {
        if(GET(HDRP(bp))!=GET(FTRP(bp)))
        {
            printf("header is not equal to footer at line %d\n",lineno);
            exit(5);
        }
        if((GET_ALLOC(HDRP(bp))!=0)&&(GET_ALLOC(HDRP(bp))!=1))
        {
            printf("alloc/free bit error at line %d\n",lineno);
            exit(6);
        }
    }

    /* Check coalescing */
    unsigned int flag=0;
    for(bp=OFST(heap_listp+2*WSIZE);GET_SIZE(HDRP(bp))>0;bp=AFTR_BLKP(bp))
    {
        if(bp==OFST(heap_listp+2*WSIZE))
            flag=GET_ALLOC(HDRP(bp));
        else
            if(flag==0&&GET_ALLOC(HDRP(bp))==0) /* two free block */
            {
                printf("coalescing error at line %d\n",lineno);
                exit(7);
            }
        flag=GET_ALLOC(HDRP(bp));
    }

    /* Check consistency and pointer range */
    for(int i=1;i<=CLASS;i++)
    {
        if(GET(i*WSIZE)!=0)
        {
            unsigned int ptr;
            for(ptr=GET(i*WSIZE);ptr!=0;ptr=NEXT_BLKP(ptr))
            {
                if(ptr<(4+CLASS)*WSIZE||ptr>mem_heapsize()-WSIZE)
                {
                    printf("list pointer range error at line %d\n",lineno);
                    exit(8);
                }
                if(GET_ALLOC(HDRP(ptr))==0&&NEXT_BLKP(ptr)!=0&&ptr!=PREV_BLKP(NEXT_BLKP(ptr)))
                {
                    printf("consistency error at line %d\n",lineno);
                    exit(9);
                }
            }
        }
    }

    /* Check the number to see if they match */
    unsigned int nlist=0;
    unsigned int nthrough=0;
    for(int i=1;i<=CLASS;i++)
    {
        if(GET(i*WSIZE)!=0)
        {
            unsigned int ptr;
            for(ptr=GET(i*WSIZE);ptr!=0;ptr=NEXT_BLKP(ptr))
                nlist++;
        }
    }
    for(bp=(CLASS+4)*WSIZE;GET_SIZE(HDRP(bp))>0;bp=AFTR_BLKP(bp))
        if(GET_ALLOC(HDRP(bp))==0)
            nthrough++;
    if(nlist!=nthrough)
    {
        printf("match error at line %d\n",lineno);
        exit(10);
    }

    /* Check the size range */
    for(int i=1;i<=CLASS;i++)
    {
        if(GET(i*WSIZE)!=0)
        {
            unsigned int ptr;
            for(ptr=GET(i*WSIZE);ptr!=0;ptr=NEXT_BLKP(ptr))
            {
                if(((GET_SIZE(HDRP(ptr)))/WSIZE)<(1u<<(i+1))||
                   (((GET_SIZE(HDRP(ptr)))/WSIZE)>=(1u<<(i+2))))
                {
                    printf("size range error at line %d\n",lineno);
                    exit(11);
                }
            }
        }
    }
}
