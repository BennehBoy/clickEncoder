#include <Arduino.h>

/********************
Arduino generic menu system
Arduino menu using clickEncoder and I2C LCD

Sep.2014 Rui Azevedo - ruihfazevedo(@rrob@)gmail.com
Feb.2018 Ken-Fitz - https://github.com/Ken-Fitz

LCD library:
https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
http://playground.arduino.cc/Code/LCD3wires
*/
#define OLED_MOSI   PA7   // SPI1_MOSI - connect to D1 on SSD1306
#define OLED_CLK    PA5   // SPI1_SCK - connect to D0 on SSD1306
#define OLEDCS_1    PB5
#define OLED_RESET  PB3
#define OLED_DC     PA15
#define fontX 5
#define fontY 9

#include <menu.h>//menu macros and objects
#include <ClickEncoder.h>
#include <menuIO/clickEncoderIn.h>
#include <menuIO/keyIn.h>
#include <menuIO/chainStream.h>
#include <menuIO/serialOut.h>
#include <menuIO/serialIn.h>
#include <menuIO/adafruitGfxOut.h>
  #include <Adafruit_SSD1306.h>
  Adafruit_SSD1306 display1(OLED_DC, OLED_RESET, OLEDCS_1);
using namespace Menu;

// Encoder /////////////////////////////////////
#define encA PB13
#define encB PB14
//this encoder has a button here
#define encBtn PA3


ClickEncoder clickEncoder(encA,encB,encBtn,2);
ClickEncoderStream encStream(clickEncoder,1);
MENU_INPUTS(in,&encStream);
// Timer STM32
#define TIMER_ENC  TIM4
//volatile uint16_t count = 0;
static stimer_t TimHandle;
static void checkenc(stimer_t *htim) {
  UNUSED(htim);
  clickEncoder.service();
}


//void timerIsr() {clickEncoder.service();}

result doAlert(eventMask e, prompt &item);

result showEvent(eventMask e,navNode& nav,prompt& item) {
  Serial.print("event: ");
  Serial.println(e);
  return proceed;
}

int test=55;

result action1(eventMask e,navNode& nav, prompt &item) {
  Serial.print("action1 event: ");
  Serial.print(e);
  Serial.println(", proceed menu");
  Serial.flush();
  return proceed;
}

result action2(eventMask e,navNode& nav, prompt &item) {
  Serial.print("action2 event: ");
  Serial.print(e);
  Serial.println(", quiting menu.");
  Serial.flush();
  return quit;
}

int ledCtrl=LOW;

result myLedOn() {
  ledCtrl=HIGH;
  return proceed;
}
result myLedOff() {
  ledCtrl=LOW;
  return proceed;
}

TOGGLE(ledCtrl,setLed,"Led: ",doNothing,noEvent,noStyle//,doExit,enterEvent,noStyle
  ,VALUE("On",HIGH,doNothing,noEvent)
  ,VALUE("Off",LOW,doNothing,noEvent)
);

int selTest=0;
SELECT(selTest,selMenu,"Select",doNothing,noEvent,noStyle
  ,VALUE("Zero",0,doNothing,noEvent)
  ,VALUE("One",1,doNothing,noEvent)
  ,VALUE("Two",2,doNothing,noEvent)
);

int chooseTest=-1;
CHOOSE(chooseTest,chooseMenu,"Choose",doNothing,noEvent,noStyle
  ,VALUE("First",1,doNothing,noEvent)
  ,VALUE("Second",2,doNothing,noEvent)
  ,VALUE("Third",3,doNothing,noEvent)
  ,VALUE("Last",-1,doNothing,noEvent)
);

//customizing a prompt look!
//by extending the prompt class
class altPrompt:public prompt {
public:
  altPrompt(constMEM promptShadow& p):prompt(p) {}
  Used printTo(navRoot &root,bool sel,menuOut& out, idx_t idx,idx_t len,idx_t) override {
    return out.printRaw(F("special prompt!"),len);;
  }
};

MENU(subMenu,"Sub-Menu",showEvent,anyEvent,noStyle
  ,OP("Sub1",showEvent,anyEvent)
  ,OP("Sub2",showEvent,anyEvent)
  ,OP("Sub3",showEvent,anyEvent)
  ,altOP(altPrompt,"",showEvent,anyEvent)
  ,EXIT("<Back")
);

/*extern menu mainMenu;
TOGGLE((mainMenu[1].enabled),togOp,"Op 2:",doNothing,noEvent,noStyle
  ,VALUE("Enabled",enabledStatus,doNothing,noEvent)
  ,VALUE("disabled",disabledStatus,doNothing,noEvent)
);*/

char* constMEM hexDigit MEMMODE="0123456789ABCDEF";
char* constMEM hexNr[] MEMMODE={"0","x",hexDigit,hexDigit};
char buf1[]="0x11";

MENU(mainMenu,"Main menu",doNothing,noEvent,wrapStyle
  ,OP("OpA",action1,anyEvent)
  ,OP("OpB",action2,enterEvent)
  //,SUBMENU(togOp)
  ,FIELD(test,"Test","%",0,100,10,1,doNothing,noEvent,wrapStyle)
  ,SUBMENU(subMenu)
  ,SUBMENU(setLed)
  ,OP("LED On",myLedOn,enterEvent)
  ,OP("LED Off",myLedOff,enterEvent)
  ,SUBMENU(selMenu)
  ,SUBMENU(chooseMenu)
  ,OP("Alert test",doAlert,enterEvent)
  ,EDIT("Hex",buf1,hexNr,doNothing,noEvent,noStyle)
  ,EXIT("<Back")
);

#define MAX_DEPTH 2

/*constMEM panel panels[] MEMMODE={{0,0,16,2}};
navNode* nodes[sizeof(panels)/sizeof(panel)];
panelsList pList(panels,nodes,1);
idx_t tops[MAX_DEPTH];
lcdOut outLCD(&lcd,tops,pList);//output device for LCD
menuOut* constMEM outputs[] MEMMODE={&outLCD};//list of output devices
outputsList out(outputs,1);//outputs list with 2 outputs
*/

const colorDef<uint16_t> colors[] MEMMODE={
  {{BLACK,WHITE},{BLACK,WHITE,WHITE}},//bgColor
  {{WHITE,BLACK},{WHITE,BLACK,BLACK}},//fgColor
  {{WHITE,BLACK},{WHITE,BLACK,BLACK}},//valColor
  {{WHITE,BLACK},{WHITE,BLACK,BLACK}},//unitColor
  {{WHITE,BLACK},{BLACK,BLACK,BLACK}},//cursorColor
  {{WHITE,BLACK},{BLACK,WHITE,WHITE}},//titleColor
};

MENU_OUTPUTS(out,MAX_DEPTH
  //,LCD_OUT(lcd,{0,0,20,4})
  ,ADAGFX_OUT(display1,colors,fontX,fontY,{0,0,128/fontX,64/fontY})
  ,NONE
);
NAVROOT(nav,mainMenu,MAX_DEPTH,in,out);//the navigation root object

result alert(menuOut& o,idleEvent e) {
  if (e==idling) {
    o.setCursor(0,0);
    o.print("alert test");
    o.setCursor(0,1);
    o.print("[select] to continue...");
  }
  return proceed;
}

result doAlert(eventMask e, prompt &item) {
  nav.idleOn(alert);
  return proceed;
}

result idle(menuOut& o,idleEvent e) {
  switch(e) {
    case idleStart:o.print("suspending menu!");break;
    case idling:o.print("suspended...");break;
    case idleEnd:o.print("resuming menu.");break;
  }
  return proceed;
}

void setup() {
  pinMode(encBtn,INPUT_PULLUP);
  pinMode(LED_BUILTIN,OUTPUT);
  pinMode (OLEDCS_1, OUTPUT);
  digitalWrite(OLEDCS_1, HIGH);
  display1.begin(8000000); //construct our displays  
  Serial.begin(9600);
  while(!Serial);
  Serial.println("Arduino Menu Library");Serial.flush();
//  lcd.begin(20,4);
  nav.idleTask=idle;//point a function to be used when menu is suspended
  mainMenu[1].enabled=disabledStatus;
  nav.showTitle=false;
    display1.setTextColor(WHITE);
    display1.setTextSize(1);
  display1.clearDisplay();
  display1.setCursor(0, 0);
  display1.print("Menu 4.x LCD");
  display1.setCursor(0, 1);
  display1.print("r-site.net");
  display1.display();
//Timer1.initialize(1000);
//  Timer1.attachInterrupt(timerIsr);
  /* Set TIMx instance. */
  TimHandle.timer = TIMER_ENC;
  /* Timer set to 1ms */
  TimerHandleInit(&TimHandle, 1000 - 1, ((uint32_t)(getTimerClkFreq(TIMER_ENC) / (1000000)) - 1));
  attachIntHandle(&TimHandle, checkenc);
  delay(2000);
}

void loop() {
  nav.poll();
  nav.doOutput();
  display1.display();
//  display1.clearDisplay();
  digitalWrite(LED_BUILTIN, ledCtrl);
  delay(100);//simulate a delay as if other tasks are running
}
