#include <M5Unified.h>

#define ENC_A_PIN   32   // Encoder line A - Right
#define ENC_B_PIN   33   // Encoder line B - Left
#define BTN_PIN     26   // Encoder button line - Press
// GND M5Stick  <->  GND CyberSafe

#define MODE_QUADRATURE   1

#define INVERT_DIR        0


#define T_STATE     4     // hold time for each quadrature state
#define T_STEP      25    // pause between rotation steps
#define T_PRESS     40    // button hold duration
#define T_AFTER_PRESS   120   // pause after press
#define T_AFTER_ATTEMPT 450   // pause after the 4th digit


#define START_CODE   0



static inline void lineAssert(uint8_t pin) {
  pinMode(pin, OUTPUT);
  digitalWrite(pin, LOW);
}
static inline void lineRelease(uint8_t pin) { 
  pinMode(pin, INPUT);
}
static inline void del(uint16_t ms) { delay(ms); }

void encoderStep(bool cw) {
#if MODE_QUADRATURE
  if (cw) {
    lineAssert(ENC_A_PIN);  del(T_STATE);
    lineAssert(ENC_B_PIN);  del(T_STATE);
    lineRelease(ENC_A_PIN); del(T_STATE);
    lineRelease(ENC_B_PIN); del(T_STATE);
  } else {
    lineAssert(ENC_B_PIN);  del(T_STATE);
    lineAssert(ENC_A_PIN);  del(T_STATE);
    lineRelease(ENC_B_PIN); del(T_STATE);
    lineRelease(ENC_A_PIN); del(T_STATE);
  }
#else
  // Simple pulse on one direction line
  uint8_t p = cw ? ENC_A_PIN : ENC_B_PIN;
  lineAssert(p);  del(T_STATE);
  lineRelease(p); del(T_STATE);
#endif
}

// Rotate right
void rotateRight(uint8_t n) {
  bool cw = (INVERT_DIR == 0);
  for (uint8_t i = 0; i < n; i++) {
    encoderStep(cw);
    del(T_STEP);
  }
}

// Press the encoder button
void pressButton() {
  lineAssert(BTN_PIN);
  del(T_PRESS);
  lineRelease(BTN_PIN);
}

// Enter a single 4-digit code
void enterCode(uint16_t code) {
  uint8_t d[4] = {
    (uint8_t)((code / 1000) % 10),
    (uint8_t)((code / 100)  % 10),
    (uint8_t)((code / 10)   % 10),
    (uint8_t)( code         % 10)
  };
  for (uint8_t i = 0; i < 4; i++) {
    rotateRight(d[i]);   // rotate from 0 up to the required digit
    del(T_STEP);
    pressButton();       // confirm
    del(T_AFTER_PRESS);
  }
  del(T_AFTER_ATTEMPT);
}

//UI

uint16_t current = START_CODE;
bool paused = false;

void drawStatus() {
  M5.Display.fillScreen(TFT_BLACK);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.setCursor(6, 8);
  M5.Display.setTextSize(2);
  M5.Display.println("CyberSafe BF");
  M5.Display.setCursor(6, 40);
  M5.Display.setTextColor(TFT_GREEN, TFT_BLACK);
  M5.Display.setTextSize(4);
  M5.Display.printf("%04u", current);
  M5.Display.setTextSize(2);
  M5.Display.setCursor(6, 95);
  M5.Display.setTextColor(paused ? TFT_YELLOW : TFT_CYAN, TFT_BLACK);
  M5.Display.println(paused ? "PAUSED" : "RUNNING");
  M5.Display.setCursor(6, 125);
  M5.Display.setTextColor(TFT_DARKGREY, TFT_BLACK);
  M5.Display.setTextSize(1);
  M5.Display.println("BtnA: pause/resume");
}

void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(0);

  lineRelease(ENC_A_PIN);
  lineRelease(ENC_B_PIN);
  lineRelease(BTN_PIN);

  drawStatus();
  delay(1500);  // time to connect wires / read the screen
}

void loop() {
  M5.update();

  // Button A: pause/resume
  if (M5.BtnA.wasPressed()) {
    paused = !paused;
    drawStatus();
  }

  if (paused) {
    delay(20);
    return;
  }

  if (current > 9999) {
    paused = true;
    M5.Display.fillScreen(TFT_BLACK);
    M5.Display.setCursor(6, 50);
    M5.Display.setTextColor(TFT_ORANGE, TFT_BLACK);
    M5.Display.setTextSize(2);
    M5.Display.println("DONE 0..9999");
    return;
  }

  drawStatus();
  enterCode(current);   // enter the next code

  current++;
}