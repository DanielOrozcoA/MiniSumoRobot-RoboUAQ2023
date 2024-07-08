#include<18f2550.h>
#device adc = 8
#fuses hs, nowdt, noprotect, nolvp, cpudiv1, pll1 //ultimo agrgado
#use delay(internal = 8M)
#use rs232(rcv = pin_c7, xmit = pin_c6, baud = 9600, stream = bt)
#priority int_timer1, int_rda

byte dato_bt = 0;
int8 entradaH = 0;
int8 pwm1 = 0;
int8 pwm2 = 0;
int8 pot = 100;
int8 cont = 0;
int8 cont2 = 0;
int8 led = 0x00;
float lec_9v = 0;
float distancia = 0;
float tiempo = 0;

int1 flag_auto = 0;
int1 flag_datos = 0;

#int_rda
void i1()
{
   led = 0x03 | led;
   dato_bt = fgetc(bt);
   switch(dato_bt)
   {
      case 'a': entradaH = 0x05; pwm1 = pot/2; pwm2 = pot; break; // diagonal izq
      case 'b': entradaH = 0x05; pwm1 = pot; pwm2 = pot; break; // adelante
      case 'c': entradaH = 0x05; pwm1 = pot; pwm2 = pot/2; break; // diagonal der
      case 'd': entradaH = 0x04; pwm1 = 0; pwm2 = pot; break; // izquierda
      case 'e': entradaH = 0x00; pwm1 = 0; pwm2 = 0; break; // alto
      case 'f': entradaH = 0x01; pwm1 = pot; pwm2 = 0; break; // derecha
      case 'g': entradaH = 0x0A; pwm1 = pot; pwm2 = pot/2; break; // rev diagonal izq
      case 'h': entradaH = 0x0A; pwm1 = pot; pwm2 = pot; break; // atras
      case 'i': entradaH = 0x0A; pwm1 = pot/2; pwm2 = pot; break; // rev diagonal der
      case 'j': pot = 100; led = 0x07; break; // vel 1
      case 'k': pot = 170; led = 0x0B; break; // vel 2
      case 'l': pot = 240; led = 0x13; break; // vel 3
      case 'm': flag_auto = 1; break; // Modo automático ON
      case 'n': flag_auto = 0; entradaH=0; pwm1=0; pwm2=0; break; // Modo automático OFF
      case 'o': flag_datos = 1; break; // Datos ON
      case 'p': flag_datos = 0; break; // Datos OFF
      default: break;
   }
   cont = 0;
}

#int_timer1
void conteo_timer1()
{
   cont++;
   if(cont >= 10)
   {
      led = 0x01;
      cont = 0;
   }
   set_timer1(15536);
}

void main()
{
   ////////// Set Tris //////////
   set_tris_a(0b11100000);
   set_tris_b(0x00);
   ////////// Interrupciones //////////
   enable_interrupts(int_rda);
   enable_interrupts(int_timer1);
   enable_interrupts(global);
   ////////// PWM //////////
   setup_ccp1(ccp_pwm);
   setup_ccp2(ccp_pwm);
   setup_timer_2(t2_div_by_16,255,1);
   ////////// Timer1 //////////
   setup_timer_1(T1_INTERNAL|T1_DIV_BY_8);
   set_timer1(15536);
   ////////// Timer0 //////////
   setup_timer_0(RTCC_INTERNAL|RTCC_DIV_2);
   ////////// ADC //////////
   setup_adc(adc_clock_internal);
   setup_adc_ports(AN0_TO_AN4|vss_vdd);
   set_tris_a(0b11100000);
   set_adc_channel(4);
   ////////// Leds //////////
   for(int i = 1; i<17; i=i<<1)
   {
      output_a(i);
      delay_ms(200);
   }
   delay_ms(200);
   output_a(0x00);
   output_high(pin_a0);
   
   while(true)
   {
      output_b(entradaH);
      output_a(led);
      set_pwm1_duty(pwm1);
      set_pwm2_duty(pwm2);
      ///// Lactura voltaje /////
      set_tris_a(0b11100000);//
      set_adc_channel(4);//
      delay_us(10);//
      lec_9v = read_adc(pin_a5)*25.0/255.0;
      ///// Lectura ultrasonico /////
      output_high(pin_b6);
      delay_us(10);
      output_low(pin_b6);
      
      while(!input(pin_b7))
      {}
      set_timer0(0);
      
      while(input(pin_b7))
      {}
      tiempo=get_timer0();
      distancia=(tiempo)/(29.15*2);
      
      ///// Condiciones para impresión de datos /////
      if(flag_datos == 1)
      {
         if(cont == 10)
         {
            fprintf(bt,"%1.2fx%2.2f",lec_9v,distancia);
            cont = 0;
         }
         cont++;
      }
      //if(flag_datos == 1) fprintf(bt,"%1.2fx%2.2f",lec_9v,distancia);
      delay_ms(100);
      
      ///// Automático /////
      if(flag_auto == 1)
      {
         if(distancia < 10.0)
         {
            pot = 240; led = 0x13; // Velocidad máxima
            entradaH = 0x05; pwm1 = pot;pwm2 = pot; // Hacia adelante
         }
         else
         {
            pot = 100; led = 0x07; // Velocidad minima
            entradaH = 0x01; pwm1 = pot;pwm2 = 0; // Derecha
         }
      }
   }
}
