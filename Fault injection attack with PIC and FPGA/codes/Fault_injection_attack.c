sbit button at PORTB.B0;
sbit power at PORTB.B1;

int main(){
    int i;
    TRISB = 0b00000001;
    PORTB = 0;
    power = 1;
    while(1){
      if(button){
        power = 0;
        asm{
            nop; //500nS
            nop; //750nS
            nop;
            nop;
            nop;
            nop;
            nop;
            nop; //2250nS
        }
        power = 1;
        Delay_ms(400);
      }
      else{
        power = 1;
      }
    }
}