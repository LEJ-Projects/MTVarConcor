/**********************************************************************
**
** Program:   MTVarConcor.cpp
**
** Purpose:  Reads a BASIC file and lists the variables and the line 
**           numbers where they are found. 
**           that reference them.
**
** Author: L. Johnson,
** Created: 14 Apr 23
** Current Version: 1.0
***********************************************************************
**  Revision   Date       Engineer       Description of Change
**  --------   --------   ------------   ------------------------------
**  1.0        30 Apr 23   L. Johnson     Initial Release 
**  1.1        02 May 23   L. Johnson     Handle ctl-z file terminator                                  
**********************************************************************/
#define TRUE  1
#define FALSE 0

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
//using namespace std;
void cont(void);
int main(int argc, char** argv) {

FILE  *fi, *fo;
  int i,j;                         /* Loop index */
  int done=0;
  char fi_name[256];      // Input file name
  char fo_name[256];      // Output file name
  char str_in[256];       // Input String
  char wrk_str[256];      // Working Sttring (Input string less spaces)
  char ln_str[256];       // Line  string
  char tmpstr[256];       // Temporary string
  int numq;               // Number of double quotes in a line
  int qi;                 // Quote index (increments on end quotes)
  int qfnd;		// Quote Found flag

  int los;  // Length of string
  int ns;   // number of items scanned


  int remF;   // Remark Flag...Set to TRUE if a remark found otherwise FALSE
  int delSp=TRUE;   // Delete space flag set to FALSE
  char cc;   // current character
  int kwcc;  // Number of Keyword characters found...

    // Keyword Parameters
  int p2i;        // Pass 2 char increment (only 1 pass now)
 
  int vct=0;  // Variable count total
  int vf;  // Variable Found Flag
  int nf;  // Number found flag
  int vnf=FALSE;  // Variable Name Found flag
  char vn[3000][20];   // Up to 3000 variables with up to 20 character names
  int vi[3000];  // Variable indices...
  int lval;    // Value of line number being processed
  int vin[5000];  // Array of variable indices to name
  int vil[5000];  // Array of variable indices to line number
  int vit=0;        // Variable Index Total
  int vlct;   // Variable line count total (the number of times a
              // variable is referenced on the same line
  int cln;  // Current Line number
  int temp;  // Used for sorting
  int vx;   // Variable index



  printf("MTVarConcor - Version 1.1\n");


  printf("Enter input file name: ");
  scanf("%s",fi_name);

  fi = fopen(fi_name,"rb");
  if (fi==NULL) {
    printf("Unable to open %s for input.\n",fi_name);
    cont();
	exit(1);
  }


  printf("Enter output file name: ");
  scanf("%s",fo_name);
  fo = fopen(fo_name,"w");
  if (fo==NULL) {
    printf("Unable to open %s for output.\n",fo_name);
    cont();
    exit(1);
  }

// 
  done=0;
  while (done == 0) {
    if (fgets(str_in,256,fi) == NULL) done=1;
    if (strlen(str_in)<3) done=1; // nothing much in this line so we quit
    if (str_in[0]==26) done=1;  // A control-Z will terminate this input file
    if (done==0) {
      // Remove carriage return & linefeed
      los=strlen(str_in);  // length of input string
      for (j=0;j<los;j++) {
        if((str_in[j]==13) ||(str_in[j]==10)) str_in[j]=0;
      }
      los=strlen(str_in);  // length of input string
      ns=sscanf(str_in,"%d %s",&lval, ln_str);
      strcpy(str_in,strstr(str_in,ln_str));   // basically the string without the line number
      los=strlen(str_in);  // length of input string

      // strip spaces ( & convert all text to upper case
      //  (except for remarks and data statements)
      qfnd=FALSE;
      numq=0;
      remF=FALSE;
      j=0;  // wrk_str index
      for (i=0;i<los;i++){
        if (str_in[i] =='"') {  //The case for quotes
          numq++;
          wrk_str[j]=' ';  //.. Convert the quote to a space.
          j++;
        } else {  // The case for non quotes
          if ((numq%2)>0) {  // We are inside a quote, do nothing 
            //wrk_str[j]=str_in[i];
            // j++;
          } 
          if ((numq%2)==0) {  // Outside quotes, we got work to do...
            //  First check for comments (treat Data statements like comments ...
            if ((((str_in[i] & 0xdf) =='R') &&  
              ((str_in[i+1] & 0xdf) =='E') &&
              ((str_in[i+2] & 0xdf) =='M'))  ||
              (str_in[i] == 0x27)  || 
              ((str_in[i]  =='D') &&  
              (str_in[i+1] =='A') &&
              (str_in[i+2] =='T') &&
              (str_in[i+3] =='A'))) remF=TRUE;           
            if (remF==TRUE) {   // It is a remark or a DATA statement, terminate the string.
			  wrk_str[j]=' ';
			  wrk_str[j+1]=0;
              i=los;
            }
            if (remF==FALSE) {   // It is not a remark - process the character
              if ((str_in[i]>96)  && (str_in[i]<123)) str_in[i]=str_in[i]-32;
              if ((str_in[i] != ' ') || (delSp==FALSE)) 
              { // if not a space, copy to the working string
                wrk_str[j]=str_in[i];
                j++;
              } // if (str_in[i] != ' ') {  // if not a space, copy to the working string
            } // if (remF==FALSE) {   // It is not a remark - process the character
          }  // if ((numq%2)==0) {  // Outside quotes, we got work to do...
        }  // if (str_in[i] =='"')  (else portion) - the case for non quotes
      } //      for (i=0;i<los;i++){
      if (remF==FALSE) wrk_str[j]=0;   // Terminate string for non remarks...


      // Convert double quotes and text between double quote pairs into spaces.
      numq=0;
      qi=0;
      qfnd=FALSE;
      for (i=0;i<(int)strlen(wrk_str);i++){
        if (wrk_str[i] =='"') {
          wrk_str[i]=' ';
          numq++;
        } // if (ln_str[i] =='"') {
        if ((numq%2)>0) wrk_str[i]=' ';
      }  //for (i=0;i<strlen(wrk_str);i++){

 
      // Take care of the rest...
      for (p2i=0;p2i<(int) strlen(wrk_str);p2i++) {
        //  Convert all 1 charcter symbols to a space
        cc=wrk_str[p2i];   // A little easier to work with shorter variable name
        if ((cc=='+') || (cc=='-')|| (cc=='*') || (cc=='/') || (cc=='\\')) wrk_str[p2i]=' ';
        if ((cc=='^') || (cc=='(') ||(cc==')') || (cc=='<') || (cc=='>'))  wrk_str[p2i]=' ';
        if ((cc==':') || (cc=='=') || (cc==',') || (cc==';') || (cc=='?') ||(cc=='@'))  
            wrk_str[p2i]=' ';        
    
        kwcc=0;  //Initialize key word character count to 0
        //  Convert all 7 character keywords to a space
        if ((kwcc==0) && 
          ((strncmp(&wrk_str[p2i],"RESTORE",7) ==0) ||
          (strncmp(&wrk_str[p2i],"STRING$",7) ==0) 
          )) kwcc=7;
        //  Convert all 6 character keywords to a space
        if ((kwcc==0) && 
          ((strncmp(&wrk_str[p2i],"CLOADM",6) ==0) ||
          (strncmp(&wrk_str[p2i],"CSAVEM",6) ==0) ||
          (strncmp(&wrk_str[p2i],"CSRLIN",6) ==0) ||
          (strncmp(&wrk_str[p2i],"DEFDBL",6) ==0) ||
          (strncmp(&wrk_str[p2i],"DEFINT",6) ==0) ||
          (strncmp(&wrk_str[p2i],"DEFSNG",6) ==0) ||
          (strncmp(&wrk_str[p2i],"DEFSTR",6) ==0) ||
          (strncmp(&wrk_str[p2i],"INKEY$",6) ==0) ||
          (strncmp(&wrk_str[p2i],"INPUT$",6) ==0) ||
          (strncmp(&wrk_str[p2i],"INPUT#",6) ==0) ||
          (strncmp(&wrk_str[p2i],"MAXRAM",6) ==0) ||
          (strncmp(&wrk_str[p2i],"LPRINT",6) ==0) ||
          (strncmp(&wrk_str[p2i],"PRESET",6) ==0) ||
          (strncmp(&wrk_str[p2i],"PRINT#",6) ==0) ||
          (strncmp(&wrk_str[p2i],"RESUME",6) ==0) ||
          (strncmp(&wrk_str[p2i],"RETURN",6) ==0) ||
          (strncmp(&wrk_str[p2i],"RIGHT$",6) ==0) ||
          (strncmp(&wrk_str[p2i],"SCREEN",6) ==0) ||
          (strncmp(&wrk_str[p2i],"SPACE$",6) ==0) ||
          (strncmp(&wrk_str[p2i],"TIMES$",6) ==0) ||
          (strncmp(&wrk_str[p2i],"VARPTR",6) ==0) 
          )) kwcc=6;
        //  Convert all 5 character keywords to a space
        if ((kwcc==0) && 
          ((strncmp(&wrk_str[p2i],"CLEAR",5) ==0) ||
          (strncmp(&wrk_str[p2i],"CLOAD",5) ==0) ||
          (strncmp(&wrk_str[p2i],"CLOSE",5) ==0) ||
          (strncmp(&wrk_str[p2i],"CSAVE",5) ==0) ||
          (strncmp(&wrk_str[p2i],"DATE$",5) ==0) ||
          (strncmp(&wrk_str[p2i],"ERROR",5) ==0) ||
          (strncmp(&wrk_str[p2i],"FILES",5) ==0) ||
          (strncmp(&wrk_str[p2i],"GOSUB",5) ==0) ||
          (strncmp(&wrk_str[p2i],"HIMEN",5) ==0) ||
          (strncmp(&wrk_str[p2i],"INPUT",5) ==0) ||
          (strncmp(&wrk_str[p2i],"INSTR",5) ==0) ||
          (strncmp(&wrk_str[p2i],"LEFT$",5) ==0) ||
          (strncmp(&wrk_str[p2i],"LABEL",5) ==0) ||
          (strncmp(&wrk_str[p2i],"LCOPY",5) ==0) ||
          (strncmp(&wrk_str[p2i],"LLIST",5) ==0) ||
          (strncmp(&wrk_str[p2i],"LOADM",5) ==0) ||
          (strncmp(&wrk_str[p2i],"MERGE",5) ==0) ||
          (strncmp(&wrk_str[p2i],"MOTOR",5) ==0) ||
          (strncmp(&wrk_str[p2i],"POWER",5) ==0) ||
          (strncmp(&wrk_str[p2i],"PRINT",5) ==0) ||
          (strncmp(&wrk_str[p2i],"SAVEM",5) ==0) ||
          (strncmp(&wrk_str[p2i],"TIME$",5) ==0) ||
          (strncmp(&wrk_str[p2i],"USING",5) ==0) 
          )) kwcc=5;
        //  Convert all 4 character keywords to a space
        if ((kwcc==0) && 
          ((strncmp(&wrk_str[p2i],"BEEP",4) ==0) ||
          (strncmp(&wrk_str[p2i],"CALL",4) ==0) ||
          (strncmp(&wrk_str[p2i],"CDBL",4) ==0) ||
          (strncmp(&wrk_str[p2i],"CHR$",4) ==0) ||
          (strncmp(&wrk_str[p2i],"CONT",4) ==0) ||
          (strncmp(&wrk_str[p2i],"CINT",4) ==0) ||
          (strncmp(&wrk_str[p2i],"CONT",4) ==0) ||
          (strncmp(&wrk_str[p2i],"CSNG",4) ==0) ||
          (strncmp(&wrk_str[p2i],"DAY$",4) ==0) ||
          (strncmp(&wrk_str[p2i],"EDIT",4) ==0) ||
          (strncmp(&wrk_str[p2i],"ELSE",4) ==0) ||
          (strncmp(&wrk_str[p2i],"GOTO",4) ==0) ||
          (strncmp(&wrk_str[p2i],"KILL",4) ==0) ||
          (strncmp(&wrk_str[p2i],"LINE",4) ==0) ||
          (strncmp(&wrk_str[p2i],"LIST",4) ==0) ||
          (strncmp(&wrk_str[p2i],"LOAD",4) ==0) ||
          (strncmp(&wrk_str[p2i],"MENU",4) ==0) ||
          (strncmp(&wrk_str[p2i],"MID$",4) ==0) ||
          (strncmp(&wrk_str[p2i],"NAME",4) ==0) ||
          (strncmp(&wrk_str[p2i],"NEXT",4) ==0) ||
          (strncmp(&wrk_str[p2i],"OPEN",4) ==0) ||
          (strncmp(&wrk_str[p2i],"PEEK",4) ==0) ||
          (strncmp(&wrk_str[p2i],"POKE",4) ==0) ||
          (strncmp(&wrk_str[p2i],"PSET",4) ==0) ||
          (strncmp(&wrk_str[p2i],"READ",4) ==0) ||
          (strncmp(&wrk_str[p2i],"RUNM",4) ==0) ||
          (strncmp(&wrk_str[p2i],"THEN",4) ==0) ||
          (strncmp(&wrk_str[p2i],"TIME",4) ==0) ||
          (strncmp(&wrk_str[p2i],"SAVE",4) ==0) ||
          (strncmp(&wrk_str[p2i],"STEP",4) ==0) ||
          (strncmp(&wrk_str[p2i],"STOP",4) ==0) ||
          (strncmp(&wrk_str[p2i],"STR$",4) ==0) 
          )) kwcc=4;
        //  Convert all 3 character keywords to a space
        if ((kwcc==0) && 
          ((strncmp(&wrk_str[p2i],"ASC",3) ==0) ||
          (strncmp(&wrk_str[p2i],"ABS",3) ==0) ||
          (strncmp(&wrk_str[p2i],"ATN",3) ==0) ||
          (strncmp(&wrk_str[p2i],"CLS",3) ==0) ||
          (strncmp(&wrk_str[p2i],"COM",3) ==0) ||
          (strncmp(&wrk_str[p2i],"COS",3) ==0) ||
          (strncmp(&wrk_str[p2i],"DIM",3) ==0) ||            
          (strncmp(&wrk_str[p2i],"END",3) ==0) ||     
          (strncmp(&wrk_str[p2i],"EOF",3) ==0) ||            
          (strncmp(&wrk_str[p2i],"ERL",3) ==0) ||            
          (strncmp(&wrk_str[p2i],"ERR",3) ==0) ||
          (strncmp(&wrk_str[p2i],"EXP",3) ==0) ||
          (strncmp(&wrk_str[p2i],"ERR",3) ==0) ||
          (strncmp(&wrk_str[p2i],"FIX",3) ==0) ||
          (strncmp(&wrk_str[p2i],"FOR",3) ==0) ||
          (strncmp(&wrk_str[p2i],"FRE",3) ==0) ||
          (strncmp(&wrk_str[p2i],"INP",3) ==0) ||
          (strncmp(&wrk_str[p2i],"INT",3) ==0) ||
          (strncmp(&wrk_str[p2i],"IPL",3) ==0) ||
          (strncmp(&wrk_str[p2i],"KEY",3) ==0) ||
          (strncmp(&wrk_str[p2i],"LEN",3) ==0) ||
          (strncmp(&wrk_str[p2i],"LET",3) ==0) ||
          (strncmp(&wrk_str[p2i],"LOG",3) ==0) ||
          (strncmp(&wrk_str[p2i],"MAX",3) ==0) ||
          (strncmp(&wrk_str[p2i],"MDM",3) ==0) ||
          (strncmp(&wrk_str[p2i],"NEW",3) ==0) ||
          (strncmp(&wrk_str[p2i],"POS",3) ==0) ||
          (strncmp(&wrk_str[p2i],"OUT",3) ==0) ||
          (strncmp(&wrk_str[p2i],"OFF",3) ==0) ||
          (strncmp(&wrk_str[p2i],"MOD",3) ==0) ||
          (strncmp(&wrk_str[p2i],"AND",3) ==0) ||
          (strncmp(&wrk_str[p2i],"XOR",3) ==0) ||
          (strncmp(&wrk_str[p2i],"EQV",3) ==0) ||
          (strncmp(&wrk_str[p2i],"IMP",3) ==0) ||
          (strncmp(&wrk_str[p2i],"NOT",3) ==0) ||
          (strncmp(&wrk_str[p2i],"RND",3) ==0) ||
          (strncmp(&wrk_str[p2i],"RUN",3) ==0) ||
          (strncmp(&wrk_str[p2i],"SGN",3) ==0) ||
          (strncmp(&wrk_str[p2i],"SIN",3) ==0) ||
          (strncmp(&wrk_str[p2i],"SQR",3) ==0) ||
          (strncmp(&wrk_str[p2i],"TAB",3) ==0) ||
          (strncmp(&wrk_str[p2i],"TAN",3) ==0) ||
          (strncmp(&wrk_str[p2i],"VAL",3) ==0) 
          )) kwcc=3;
        //  Convert all 2 character keywords to a space
        if ((kwcc==0) &&
          ((strncmp(&wrk_str[p2i],"AS",2) ==0) ||
          (strncmp(&wrk_str[p2i],"IF",2) ==0) ||
          (strncmp(&wrk_str[p2i],"ON",2) ==0) ||
          (strncmp(&wrk_str[p2i],"TO",2) ==0) ||
          (strncmp(&wrk_str[p2i],"OR",2) ==0))) kwcc=2;
        // Do the space conversion by converting key word characters into a space
       if (kwcc>0) for (j=0;j<kwcc;j++) wrk_str[p2i+j]=' ';
      } //for (p2i=0;p2i<strlen(wrk_str);p2i++) {
  
       printf("%d %s\n",lval,wrk_str);

      //  wrk_str is now just variables and numbers separated by spaces.   
      // We now want to harvest the variables but ignore the numbers.
      
      cc=0; // Current Character (of tmpstr) = 0;
      for (p2i=0;p2i<(int) strlen(wrk_str);p2i++) {
        vf=FALSE;  //  Variable Found Flag
        nf=FALSE;  // Number found flag
        if (isalpha(wrk_str[p2i]) && (nf==FALSE)) { // Found a variable
          vf=TRUE;
          while (vf==TRUE) { 
            tmpstr[cc]=wrk_str[p2i];  // Get first character
            cc++; // Increment current character
            p2i++; // Increment wrk_str index
            if ((wrk_str[p2i]==' ') || (wrk_str[p2i]==0)) {
              vf=FALSE; 
              tmpstr[cc]=0;  // Terminate tmpstr
              cc=0;  // Initialize for next character
              // Here is where it gets interesting ...
              if (vct>0) {  // Check if it is already recorded
                 vnf=FALSE;
                 for(j=0;j<vct;j++) {
                    if (strcmp(tmpstr,vn[j])== 0) {
                      vnf=TRUE;
                      vin[vit]=j;
                      vil[vit]=lval;
                      vit++;
                      j=vct;  // End search
                    } //if (strcmp(tmpstr,vn[j])== 0) {
                 }  // for(j=0;j<vct;j++) {
              }  // if (vct>0) {  // Check if it is already recorded
              if (vnf==FALSE) { // A new variable
                strcpy(vn[vct],tmpstr);  // save the variable
                vin[vit]=vct;  // Save the index
                vil[vit]=lval;  // store the line value
                vi[vct]=vct;   // Populate variable index array
                vit++;  // increment variable index total
                vct++;  // increment variable count   
              }
              p2i--;  // decrement p2i so next char is processed by loop
            } else {
              tmpstr[cc]=wrk_str[p2i]; 
            }
          }  // while (vf) { 

        }       //  if (isalpha(wrk_str[p2i]) { // Found a variable


        if (isdigit(wrk_str[p2i]) && (vf==FALSE)) { // Found a number
          nf=TRUE;
          while (nf==TRUE) { 
            p2i++; // Increment wrk_str index
            if ((wrk_str[p2i]==' ') || (wrk_str[p2i]==0)) {
              nf=FALSE; 
              p2i--;  // decrement p2i so next char is processed by loop
            } 
          }
        }       //  if (isdigit (wrk_str[p2i]) { // Found a number

      } //for (p2i=0;p2i<strlen(wrk_str);p2i++) {
    }  // if (done==0)



  } // end while

  fclose(fi);


  printf("Ready to examine data\n");
  cont();

  // First sort the variable names 
  for (i=0;i<vct-1;i++) {
    for (j=i+1;j<vct;j++) {
      if (strcmp(vn[vi[i]],vn[vi[j]])>0) {
        temp=vi[j];
        vi[j]=vi[i];
        vi[i]=temp;
      }
    }
  }

  // Now print out the variable concordance
  for (i=0;i<vct;i++) {
    vx=vi[i];  // will use the sorted indices
    printf("%s ",vn[vx]);
    fprintf(fo,"%s ",vn[vx]);
    vlct=0;
    cln=-1;  // Current line number matched
    for (j=0;j<vit;j++) {
      if (vin[j]==vx) {
        if (vil[j]==cln) vlct++;  // Simply increment
        if ((vil[j]!=cln) && (cln==-1)) {
          // The first one
          printf(" %d ",vil[j]);
          fprintf(fo," %d",vil[j]);
          vlct=1;   //Intialize variable line number count for this line numer)
          cln=vil[j];
        }
        if ((vil[j]!=cln) && (cln!=-1)) {
          // A new Line number.
          // First we will print the number of the old one if greater than 1
          if (vlct>1) {
            printf("(%d)",vlct);
            fprintf(fo,"(%d)",vlct);
          }
          // Next print the first one of the next line number
          printf(" %d",vil[j]);
          fprintf(fo," %d",vil[j]);
          vlct=1;   //Intialize variable line number count for this line numer)
          cln=vil[j];  // Save the current line number in case there are more
        }
      }
    }
    if (vlct==1) {
      printf("\n");
      fprintf(fo,"\n");
    } else {
      printf("(%d)\n",vlct);
      fprintf(fo,"(%d)\n",vlct);
    }

  }



  fclose(fo);
  printf("I guess we're done.\n");
  cont();

}
/****************************************************************
** Function:  cont()
**
** Description:  Prompts operator to press enter to continue.
**
****************************************************************/
void cont(void) {

  getchar();   /* Seems to be necessary to flush stdin */
  printf("Press enter to continue:");
  getchar();   /* This is the one that counts */
}


