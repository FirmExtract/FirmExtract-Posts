volatile unsigned char DEBUG_FLAG = 0;

volatile unsigned int i, check;

void main(){
     TRISB = 0;
     TRISA = 0;
     PORTB = 0;
     PORTA = 0;
     while(1){
       check = 0;
       for(i=0;i<=9999;i++){
         ++check;
       }
       if(check == 10000){
         PORTB = 0b11111111;
         delay_ms(1000);
         PORTB = 0;
       }
       else{
         PORTA = 0b11111111;
         delay_ms(4000);
         PORTA = 0;
       }
     }
}