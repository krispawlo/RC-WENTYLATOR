// PROJEKT STEWE
// Sterownik wentylatora zasilacza
// SQ9MDD http://sq9mdd.qrz.pl rysieklabus@gmail.com
// Udostępniam na licencji GPL
//*************************************************************************************************************
/*
 CHANGELOG
 2015.05.09 v.1.0.4 poprawki w komentarzach filtrowanie pomiaru napięcia 
 dodanie zmiennych wartości filtru temperatury i napięcia
 2015.04.23 v.1.0.3 pomiar napięcia, czyszczenie kodu 
 2015.04.16 v.1.0.2 wyświetlanie danych na LCD
 2015.04.15 v.1.0.1 zmiany pinologii obsługa wyświetlacza
*/ 
//*************************************************************************************************************

//konfiguracja podstawowa, zmienne można ostrożnie modyfikować
//#define DEBUG
#define software_version "1.0.6"
const int temp_treshold = 250;                        // temperatura startu wentylatora (st C x 10)
const int temp_max = 900;                             // temperatura przy której wentylator osiąga max obrotów
const int led1_on = 250;
const int led1_off = 250;
const int led2_on = 300;
const int led2_off = 300;
const int buzz_on = 260;
const int buzz_off = 260;

int pwm_min = 4;                                      // punkt startu wentylatora
int pwm_max = 15;                                     // maksimum wysterowania wentylatora
const unsigned long draw_interval  = 1000;            // interwał odświeżania wyświetlacza w msec
const unsigned long pomiar_temp_interval = 100;       // interwał pomiaru temperatury w msec
const unsigned long pomiar_napiecia_interval = 500;   // interwał pomiaru napięcia w msec
const int filtr_pomiar_v = 10;                        // filtr pomiaru napięcia 
const int filtr_pomiar_t = 10;                        // filtr pomiaru temperatury
const int contrast = 70;                              // kontrast wyświetlacza

//*************************************************************************************************************
//zmienne wewnetrzne poniżej tej lini proszę nic nie modyfikować
const int pomiar_temp_input = A5;           //wejście pomiaru temperatury
const int pomiar_napiecia  = A6;            //wejście pomiaru napięcia 
const int output_pin = 3;                   //pwm output sterowanie wentylatorem
const int lcd_led_pin = 13;                 //podświetlanie wyświetlacza
int went_drive = 0;                         //zmienna wysterowanie PWM dla wentylatora
unsigned long time_to_draw = 0;             //
unsigned long time_to_mesure_voltage = 0;   //
unsigned long time_to_mesure_temp = 0;      //
char buffor[] = "               ";          //
int temp = 0;                               //
int voltage = 0;                            //
int temp_buffor = 0;                        //
int volt_buffor = 0;                        //
int voltage_buffor = 0;                     //
int wysterowanie = 0;                       //

//*************************************************************************************************************
//podłączamy bibliotekę do obsługi wyświetlacza
#include <LCD5110_Graph.h>
// inicjalizujemy wyświetlacz
// lcd    - arduino
// sclk   - PIN 7
// sdin   - PIN 6
// dc     - PIN 5
// reset  - PIN 2
// sce    - PIN 4
LCD5110 myGLCD(A0,A1,A2,A4,A3); 
extern uint8_t TinyFont[];       //czcionka z biblioteki.....dodałem małe fonty (SP6IFN)
extern uint8_t SmallFont[];      //czcionka z biblioteki
extern uint8_t MediumNumbers[];  //czcionka z biblioteki

//tablica konwersji wejscie analogowe na temp, 
//ktoś zaprojektował ładną krzywą dla NTC 10k
const int temps[] PROGMEM = { 0, 1, 2, 3, 4, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 24, 25, 26, 
27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 
57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 
86, 87, 88, 88, 89, 90, 91, 92, 93, 94, 95, 96, 97, 98, 99, 100, 101, 102, 102, 103, 104, 105, 106, 107, 108, 109, 110, 
111, 112, 113, 114, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 124, 125, 126, 127, 128, 129, 130, 131, 132, 
133, 134, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143, 143, 144, 145, 146, 147, 148, 149, 150, 151, 151, 152, 153, 
154, 155, 156, 157, 158, 159, 159, 160, 161, 162, 163, 164, 165, 166, 167, 167, 168, 169, 170, 171, 172, 173, 174, 175, 
175, 176, 177, 178, 179, 180, 181, 182, 182, 183, 184, 185, 186, 187, 188, 189, 190, 190, 191, 192, 193, 194, 195, 196, 
197, 197, 198, 199, 200, 201, 202, 203, 204, 205, 205, 206, 207, 208, 209, 210, 211, 212, 212, 213, 214, 215, 216, 217, 
218, 219, 220, 220, 221, 222, 223, 224, 225, 226, 227, 228, 228, 229, 230, 231, 232, 233, 234, 235, 235, 236, 237, 238, 
239, 240, 241, 242, 243, 243, 244, 245, 246, 247, 248, 249, 250, 251, 252, 252, 253, 254, 255, 256, 257, 258, 259, 260, 
260, 261, 262, 263, 264, 265, 266, 267, 268, 269, 269, 270, 271, 272, 273, 274, 275, 276, 277, 278, 279, 279, 280, 281, 
282, 283, 284, 285, 286, 287, 288, 289, 289, 290, 291, 292, 293, 294, 295, 296, 297, 298, 299, 300, 301, 301, 302, 303, 
304, 305, 306, 307, 308, 309, 310, 311, 312, 313, 314, 315, 315, 316, 317, 318, 319, 320, 321, 322, 323, 324, 325, 326, 
327, 328, 329, 330, 331, 332, 333, 334, 335, 335, 336, 337, 338, 339, 340, 341, 342, 343, 344, 345, 346, 347, 348, 349, 
350, 351, 352, 353, 354, 355, 356, 357, 358, 359, 360, 361, 362, 363, 364, 365, 366, 367, 368, 369, 370, 371, 372, 373, 
374, 375, 376, 377, 378, 379, 380, 381, 382, 383, 384, 385, 386, 387, 388, 389, 390, 392, 393, 394, 395, 396, 397, 398, 
399, 400, 401, 402, 403, 404, 405, 406, 407, 408, 410, 411, 412, 413, 414, 415, 416, 417, 418, 419, 420, 422, 423, 424, 
425, 426, 427, 428, 429, 430, 432, 433, 434, 435, 436, 437, 438, 439, 441, 442, 443, 444, 445, 446, 448, 449, 450, 451, 
452, 453, 455, 456, 457, 458, 459, 460, 462, 463, 464, 465, 466, 468, 469, 470, 471, 472, 474, 475, 476, 477, 479, 480, 
481, 482, 484, 485, 486, 487, 489, 490, 491, 492, 494, 495, 496, 498, 499, 500, 501, 503, 504, 505, 507, 508, 509, 511, 
512, 513, 515, 516, 517, 519, 520, 521, 523, 524, 525, 527, 528, 530, 531, 532, 534, 535, 537, 538, 539, 541, 542, 544, 
545, 547, 548, 550, 551, 552, 554, 555, 557, 558, 560, 561, 563, 564, 566, 567, 569, 570, 572, 574, 575, 577, 578, 580, 
581, 583, 585, 586, 588, 589, 591, 593, 594, 596, 598, 599, 601, 603, 604, 606, 608, 609, 611, 613, 614, 616, 618, 620, 
621, 623, 625, 627, 628, 630, 632, 634, 636, 638, 639, 641, 643, 645, 647, 649, 651, 653, 654, 656, 658, 660, 662, 664, 
666, 668, 670, 672, 674, 676, 678, 680, 683, 685, 687, 689, 691, 693, 695, 697, 700, 702, 704, 706, 708, 711, 713, 715, 
718, 720, 722, 725, 727, 729, 732, 734, 737, 739, 741, 744, 746, 749, 752, 754, 757, 759, 762, 764, 767, 770, 773, 775, 
778, 781, 784, 786, 789, 792, 795, 798, 801, 804, 807, 810, 813, 816, 819, 822, 825, 829, 832, 835, 838, 842, 845, 848, 
852, 855, 859, 862, 866, 869, 873, 877, 881, 884, 888, 892, 896, 900, 904, 908, 912, 916, 920, 925, 929, 933, 938, 942, 
947, 952, 956, 961, 966, 971, 976, 981, 986, 991, 997, 1002, 1007, 1013, 1019, 1024, 1030, 1036, 1042, 1049, 1055, 1061, 
1068, 1075, 1082, 1088, 1096, 1103, 1110, 1118, 1126, 1134, 1142, 1150, 1159, 1168, 1177, 1186, 1196, 1206, 1216, 1226, 
1237, 1248, 1260, 1272, 1284, 1297, 1310, 1324, 1338, 1353, 1369, 1385, 1402, 1420, 1439, 1459, 1480, 1502 };

//*************************************************************************************************************

//funkcja piszaca po wyświetlaczu LCD no comments
void update_lcd_data(){               
 if (millis() >= time_to_draw){ 
   int a = temp/10;
   int b = temp%10;   
   myGLCD.setFont(SmallFont); 
   sprintf(buffor,"T.pom   %u.%uC",a,b);
   myGLCD.print(buffor,0,0);
   int c = temp_treshold/10;
   int d = temp_treshold%10;
   sprintf(buffor,"T.start %u.%uC",c,d);
   myGLCD.print(buffor,0,8);
   int e = temp_max/10;
   int f = temp_max%10;
   sprintf(buffor,"T.max   %u.%uC",e,f);
   myGLCD.print(buffor,0,16); 
   int g = voltage/10;
   int h = voltage%10;
   sprintf(buffor,"Vin.    %02u.%uV",g,h);
   myGLCD.print(buffor,0,40);
   sprintf(buffor,"Wyst.   %03u" ,wysterowanie);
   myGLCD.print(buffor,0,24);
   myGLCD.print("%",71,24);
   myGLCD.update(); 
   time_to_draw = millis() + draw_interval;
 } 
}

void function_pomiar_temperatury(){
  if(millis() >= time_to_mesure_temp){
    int temp_raw = analogRead(pomiar_temp_input)-238;        // odczyt danych z czujnika  
    temp = pgm_read_word(&temps[temp_raw]);                  // kalibracja miernika temp (rezystancyjny 10K) 
    temp = (temp_buffor + temp)/(filtr_pomiar_t + 1);        // przeliczenie z buforem temp                         
    temp_buffor = temp * filtr_pomiar_t;                     // przygotowanie kolejnego bufora temp
    time_to_mesure_temp = millis() + pomiar_temp_interval;   // odłożenie wykonania funkcji do kolejnego czasu pomiaru
  }
}

void function_pomiar_napiecia(){
  if(millis() >= time_to_mesure_voltage){
    voltage = map(analogRead(pomiar_napiecia),585,906,90,140);    // pomiar napięcia kalibracja dzielnika
    voltage = (volt_buffor + voltage)/(filtr_pomiar_v + 1);       //przeliczenie z buforem
    volt_buffor = voltage * filtr_pomiar_v;                       //przygotowanie nowego bufora
    time_to_mesure_voltage = millis() + pomiar_napiecia_interval; // odłożenie wykonania funkcji do kolejnego czasu pomiaru
  }  
}

void setup(){
  pinMode(pomiar_temp_input,INPUT);                        //inicjalizacja pinu pomiaru temp.
  pinMode(pomiar_napiecia,INPUT);                          //inicjalizacja pinu pomiaru napięcia
  pinMode(output_pin,OUTPUT);                              //inicjalizacja pinu sterowania
  pinMode(lcd_led_pin,OUTPUT);
  #ifdef DEBUG
    Serial.begin(9600);
  #endif
  myGLCD.InitLCD(contrast);                                //odpalamy lcd ustawiamy kontrast
  myGLCD.clrScr();                                         //czyścimy ekran z ewentualnych śmieci
  myGLCD.setFont(TinyFont);                                //czas na reklame, mała czcionka  
  myGLCD.print("STEVE ver.",0,2);                          //lokowanie produktu przy starcie
  myGLCD.print(software_version, 48,2);                    //numer wersji
  myGLCD.update(); 
  delay(3000);                                             //opoznienie   
  myGLCD.clrScr();                                         //koniec reklam czyscimy ekran
  myGLCD.update();   
  #ifdef DEBUG
    temp = temp_treshold;   
  #endif
}//end setup

void loop(){
   
    if (temp >= led1_on){ digitalWrite(5,HIGH); }
    if (temp < led1_off){ digitalWrite(5,LOW); }
if (temp >= led2_on){ digitalWrite(6,HIGH); }
    if (temp < led2_off){ digitalWrite(6,LOW); }

if (temp >= buzz_on){ tone(10,4300); }
    if (temp < buzz_off){ tone(10,LOW); }
    
    function_pomiar_temperatury();
    function_pomiar_napiecia();   
  #ifdef DEBUG
    temp++;
    delay(1000);  
  #endif
  update_lcd_data();
  //sterowanie wyjściem wentylatora
      int went_min = 255 - pwm_min;                        //odwracam końce
      int went_max = 255 - pwm_max;                        //odwracam końce
      if(temp >= temp_treshold){
        if(temp < temp_max){
          went_drive =map(temp,temp_treshold,temp_max,went_min,went_max);  //sterowanie wydatkiem wentylatora 
          went_drive = constrain(went_drive,went_max,went_min);
          wysterowanie = constrain(map(went_drive,went_min,went_max,0,100),0,100); 
          #ifdef DEBUG
            Serial.println(String(temp)+","+String(wysterowanie)+ ","+String(went_drive)+","+String(pwm_min)+","+String(pwm_max));
          #endif
        }else{
          went_drive = 0;
          wysterowanie = 100;
        }
      }else{
        went_drive = 255; //stop
        wysterowanie = 0;
      }
    analogWrite(output_pin,went_drive);                                    //sterowanie driverem wentylatora
}//end loop

//EOF
//*************************************************************************************************************


