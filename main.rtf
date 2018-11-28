#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAX_num 20
#define MAX_weight 300
#define MIN_weight 20
#define MAX_height 2.20
#define MIN_height 1.00

int n = 0;
int reset_mode = 0;
int reboot_mode = 0;

//Define a structure variables that consists of the relevant information about the input user
typedef struct
{
    int mode;                     // asian:mode=1;non-asian:mode=2
    float weight, height;         // weight， height of the input user
    float weight_chg;             // weight change recommendation
    char *status;
    float bmi;                    // BMI value
    float norm_up, over_up;       // normal weight upper limit, overweight upper limit
} person;

//Declare functions
void f_advice(person *);
void f_table(person *, int *, int *);
int f_input_check(char *);
void f_getinfo (person *);
int f_reset_check(char *);

int main()
{
    //Initialize variables
    person data[100];
    char opt[10];
    int input_weight = 0, input_height = 0, input_mode=0;
    char test_string[50];

    printf("### Program Introduction ###\n\nThis program helps to compute users' BMI by taking in mass(kg) & height(m) of individuals.\n");
    printf("It takes in particulars of all users, then output all data in a table at last.\n\n");
    printf("Note: At anytime,\n      To reset current user's data,  enter 'R'.\n      To reset all user's data,      enter 'X'.\n");

    do
    {
        // Prompt the user to state if he/she is asian
        LABEL_1:
        while (input_mode == 0)
        {
            reboot_mode = 0; reset_mode = 0;

            printf("\n(User %d)\nFor Asian,     enter '1'\nFor Non-Asian, enter '2'\n'1' or '2': ",n+1);
            scanf("%s",test_string);
            while ((getchar()) != '\n');                                        //clear buffer excess []size
            if(f_reset_check(test_string) == 1) break;                          //reset function
            else if(strlen(test_string) ==1 && (atof(test_string) == 1 || atof(test_string) == 2)){
                data[n].mode = (int)atof(test_string);
                input_mode = 1;
            }
            else {
                printf("### INVALID INPUT ###\nPlease enter either '1' or '2'.\n");
            }
        }

        // Checking for reset mode
        if(reboot_mode == 1){
            n=0; input_weight = 0; input_height = 0; input_mode=0;
            goto LABEL_1;
        }
        else if(reset_mode == 1){
            if (n==0){
                input_weight = 0; input_height = 0; input_mode=0;
                goto LABEL_1;
            }
            else {
                n--;input_weight = 0; input_height = 0; input_mode=0;
                goto LABEL_2;
            }
        }

        // Define the normal weight upper limit and overweight upper limit
        if(data[n].mode==1)
        {
            data[n].norm_up=23.00;
            data[n].over_up=27.50;
        }
        else
        {
            data[n].norm_up=24.90;
            data[n].over_up=29.90;
        }

        // Prompt the user to input his/her weight & height
        f_getinfo(&data[n]);

        // Checking for reset mode
        if(reboot_mode == 1){
            n=0; input_weight = 0; input_height = 0; input_mode=0;
            goto LABEL_1;
        }
        else if(reset_mode == 1){
            if (n==0){
                input_weight = 0; input_height = 0; input_mode=0;
                goto LABEL_1;
            }
            else {
                n--;input_weight = 0; input_height = 0; input_mode=0;
                goto LABEL_2;
            }
        }

        // Call the advice function to compute and generate relevant outputs
        f_advice(&data[n]);

        // Prompt the user to ask if he/she want to continue the program
        LABEL_2:
        printf("Enter any key to proceed to (User %d).\nEnter 'n' for CONCLUSION TABLE.\n",n+2);
        scanf("%s",opt);
        input_weight = 0; input_height = 0; input_mode=0;                           //refresh data register
        n++;                                                                        //assume getting new user data if didn't receive 'n' key from user
    } while(opt[0]!='n' || strlen(opt)!=1);

    // Generate the result table if the user choose to end the program
    for (int j=0;j<n;j++) f_table(&data[j],&j,&n);                                       //print data table of all user
    return 0;
}   //end of main


// Compute the BMI value and weight change recommended if necessary
// Categorise the weight status of the person according the BMI value
void f_advice(person *data)
{
    char *status;
    data->weight_chg=0;

    // Compute BMI value
    data->bmi = (int)(data->weight/(data->height*data->height)*100)/100.00;

    // Categorise the weight status, compute weight change recommended
    if(data->bmi<18.5)
    {
        data->status="UNDERWEIGHT";
        data->weight_chg=18.5*data->height*data->height-(data->weight);
    }
    else if(data->bmi<=data->norm_up)
    {
        data->status="NORMAL";
    }
    else if(data->bmi<=data->over_up)
    {
        data->status="OVERWEIGHT";
        data->weight_chg=23*data->height*data->height-(data->weight);
    }
    else
    {
        data->status="OBESE";
        data->weight_chg=23*data->height*data->height-(data->weight);
    }

    // Print the result of BMI value and weight status
    printf("\n(User %d)\n",n+1);
    printf("BMI value: %.2f\nBMI status: %s\n\n", data->bmi, data->status);

    // Print the weight change recommendation if necessary
    if(data->weight_chg!=0)
        printf("To achieve NORMAL status, you should %s %.2lfkg of weight\n\n",(data->weight_chg>0)?"put on":"reduce", fabs(data->weight_chg));
}


// Generate and arrange the outputs in a table
void f_table(person *data, int *j, int *n)
{
    int i=0;
    if (*j==0)
    {
        // Print a horizontal dividing line using '=' sign
        for (i=0;i<=80;i++) printf("=");
        printf("\n");

        // Print the title of each column in a row: height, weight, BMI value, weight change
        // Arrange each column so that they are equally spaced
        for (i=0;i<=18;i++) printf(" ");
        printf("Height (m)");
        for (i=0;i<=3;i++) printf(" ");
        printf("Weight (KG)");
        for (i=0;i<=3;i++) printf(" ");
        printf("BMI Value");
        for (i=0;i<=3;i++) printf(" ");
        printf("Weight to Change\n");

        // Print a horizontal dividing line using '=' sign
        for (i=0;i<=80;i++) printf("=");
        printf("\n");
    }

    // Print the result corresponding to each column title
    printf("user %d(%s)",*j+1,(data->mode==1)?"Asian":"Non-Asian");
    (data->mode==1)?printf("        "):printf("    ");
    //for (i=0;i<=5;i++) printf(" ");
    printf("%.2f",data->height);
    for (i=0;i<=10;i++) printf(" ");
    printf("%.2f",data->weight);
    for (i=0;i<=8;i++) printf(" ");
    printf("%.2f",data->bmi);
    for (i=0;i<=5;i++) printf(" ");
    printf("%c%.2f (%s)\n",(data->weight_chg>0)?'+':'-', fabs(data->weight_chg), data->status);

    // Print a table showing the range of BMI values for each weight status
    if(*j==*n-1)
    {
        printf("\n\n\n");
        for (i=0;i<=81;i++) printf("%s",(i!=81)?"=":"\n");
        printf("\t\tUnderweight\tNormal\t\tOverweight\tObese\n");
        for (i=0;i<=81;i++) printf("%s",(i!=81)?"=":"\n");
        printf("Asian\t\t<18.50\t\t18.50-23.00\t23.01-27.50\t>27.50\n");
        printf("Non-asian\t<18.50\t\t18.50-24.90\t24.91-29.90\t>29.90\n");

        for (i=0;i<=81;i++) printf("%s",(i!=81)?"=":"\n");
    }
}


void f_getinfo (person *user){
    int test_logic1 = 0;
    int test_logic2 = 0;
    char test_string[50];
    char confirm[10];

    LABEL_INFO:
    while(test_logic1 == 0 && reset_mode == 0){
        printf("\n(User %d)\nEnter weight (kg): ",n+1);
        scanf("%s",test_string);
        while ((getchar()) != '\n');                                 //clearing input buffer;
        if(f_reset_check(test_string) == 1) return;                  //reset function
        else if((f_input_check(test_string))==1 && atof(test_string) >= MIN_weight && atof(test_string) <= MAX_weight)
            test_logic1 =1;                                          //to check numeric & logical input.
        else printf("### INVALID INPUT ###\nPlease enter your weight in KiloGrams (kg).\n");
    }

    user->weight = atof(test_string);

    while(test_logic2 == 0){
        printf("\n(User %d)\nEnter height  (m): ",n+1);
        scanf("%s",test_string);
        while ((getchar()) != '\n');                                                                  //clearing input buffer;
        if(f_reset_check(test_string) == 1) return;                                                   //reset function
        else if((f_input_check(test_string))==1 && atof(test_string) >= MIN_height && atof(test_string) <= MAX_height)
            test_logic2 =1;                                         //to check numeric & logical input.
        else printf("### INVALID INPUT ###\nPlease enter your height in Metres (m).\n");
    }

    user->height = atof(test_string);

LABEL_CONFIRM:
    printf("\nPlease confirm particular entered: \n");
    printf("(User %d)\n",n+1);
    printf("Weight (kg):\t%.2f kg\n",user->weight);
    printf("Height  (m):\t %.2f m\n",user->height);
    printf("\n1.Correct\n2.Incorrect\n");
    scanf("%s",confirm);
    while ((getchar()) != '\n');
    if(f_reset_check(confirm) == 1) return;
    else if(strlen(confirm) == 1 && confirm[0] == '1') return;
    else if(strlen(confirm) == 1 && confirm[0] == '2'){
        test_logic1 = 0; test_logic2 = 0;
        goto LABEL_INFO;
    }
    else {
        printf("### INVALID INPUT ###\nEnter '1' for CORRECT,\n      '2' for INCORRECT.'\n");
        goto LABEL_CONFIRM;
    }
}


int f_input_check(char string[]){
    int test_logic=1;

    for(int i = 0; string[i] != '\0'; ++i) {
        //printf("%c",string[i]);
        if(((int)string[i] < '0' || (int)string[i] > '9') && string[i] != '.'){         //check for non-numeric input
            //printf("The string has characters that are not numbers or period!\n");
            test_logic =0;
            break;
        }
    }
    return (test_logic);
}


int f_reset_check(char *test_string){
    if(strlen(test_string) == 1 && toupper(test_string[0]) == 'R'){                               //Reset function
        reset_mode = 1;
        printf("\n### (User %d) Data Reset ###\n",n+1);
        return(1);
    }
    if(strlen(test_string) == 1 && toupper(test_string[0]) == 'X'){                               //Reset function
        reboot_mode = 1;
        printf("\n### All User Data Reset ###\n");
        return(1);
    }
    else return(0);
}
