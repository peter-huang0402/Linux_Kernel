
#include <stdio.h>
#include <string.h>

#define LOG_ENABLE 0

static inline char* sub_str(char * dest, char *src, int s_idx, int e_idx){
   #if LOG_ENABLE
   	printf("2. src=%s\t",src);
   	printf("3. &src[0]=%s \t", &src[0] );
   	printf("4. *( src+1)=%c \t", *(src+1) ); 
   	printf("5. src=%c \t", *src );   
   #endif

 
   int i=0;
   int isEmpty = 1; 
   while (  *(src+i) !='\0' && i<=e_idx  ){
    
      if ( i>= s_idx ){
          isEmpty =0;
          *(dest+(i-s_idx)) = *(src+i);
         #if LOG_ENABLE
         	printf("i=%d, dest=%s\n",i,dest);
         #endif
      }  
             
       i++;  
   } 
 
 
   if (isEmpty == 0 ){
      *(dest+(i-s_idx)) ='\0';
   }else{
      *(dest+0)= '\0';
    }

    
    #if LOG_ENABLE
       printf("dest=%s, done\n",dest);
    #endif
   return dest;
}



static inline char * asm_sub_str(char * dest, char *src,int s_idx, int e_idx) {
 int result_s, result_d , result_count , result_al ;

    __asm__ __volatile__(
            "1:\n"
            "decl %2\n\t"       //ecx(count) --
            "js 2f\n\t"         // if count<0, jump flag 2.
            "lodsb\n\t"         // load one byte from esi(src+s_idx) to AL and  esi is incremented.
            "stosb\n\t"            // store  one byte of AL   to edi(dest)
            "testb %%al,%%al\n\t"  // to test al is null 
            "jne 1b\n\t"           // if not null, jump 1, if null, go next  
            "2:"
            : "=&S" ( result_s ), "=&D" ( result_d), "=&c" ( result_count), "=&a" (result_al)   //esi,edi,ecx , al save in result_s, result_d, result_count, result_al;
           
            :"0" (src+ s_idx),"1" (dest),"2" ( e_idx - s_idx +1 ) : "memory"); // 0--> assign (src+s_idx) with the first menitioned register, in the case is esi (result_s)
                
  
            #if LOG_ENABLE
                printf("result_s=%d, result_d=%d , result_count=%d, result_al=%c, \n",  result_s , result_d, result_count, result_al);
           #endif
 
     return dest; 
 } 



 int main(int argc, char **argv){
       
       if (argc != 4){
          printf("Your Format you key in is wrong\nPlease key in with these paramters like\n [words] [start_index] [end_index] like abcdefghijk  0 5 \n");
          return -1;
       }     
     
       int s = atoi(argv[2]);
       int e =atoi(argv[3]);

       if (e -s <0 ){
          printf("Invalid index, Your end_index is not less than start_index!\n");
          return -1;
       }

        char* d = (char*)malloc(sizeof(char) * (e-s+1) );  
  
  
      printf("argc=%d, src=%s, s=%d, e=%d\n",argc, argv[1],s,e);	
      char* d1 = sub_str(d, argv[1], s ,e );
      printf( "c code: d1=%s, d=%s, finish!\n", d1,d);    
     
    
      char* new_d = (char*)malloc(sizeof(char) * (e-s+1) );  
      char* d2 = asm_sub_str(new_d, argv[1],s,e);
      printf("inline asm: d2=%s, d=%s, done\n",d2,d );          
    

 
      if (strcmp(d1,d2) == 0){
          printf("d1 & d2 are equal.\n");
          return 1;
      }else{
         printf("d1 & d2 are not equal.\n");
         return -1;
     }     

  
}
