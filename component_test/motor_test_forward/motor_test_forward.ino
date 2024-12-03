// Configuración de pines
#define PIN_Motor_STBY 3
#define PIN_Motor_AIN_1 7
#define PIN_Motor_PWMA 5
#define PIN_Motor_BIN_1 8
#define PIN_Motor_PWMB 6

void setup() {
  // Configurar los pines como salidas
  pinMode(PIN_Motor_STBY, OUTPUT);
  pinMode(PIN_Motor_AIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMA, OUTPUT);
  pinMode(PIN_Motor_BIN_1, OUTPUT);
  pinMode(PIN_Motor_PWMB, OUTPUT);

  // Activar el modo "Standby" para habilitar los motores
  digitalWrite(PIN_Motor_STBY, HIGH);
}

void loop() {
  // Configurar ambos motores para avanzar en línea recta

  // Motor A (lado derecho)
  digitalWrite(PIN_Motor_AIN_1, HIGH); // Avanzar
  analogWrite(PIN_Motor_PWMA, 255);   // Velocidad (ajusta según sea necesario)

  // Motor B (lado izquierdo)
  digitalWrite(PIN_Motor_BIN_1, HIGH); // Avanzar
  analogWrite(PIN_Motor_PWMB, 255);   // Velocidad (ajusta según sea necesario)

  // El robot se moverá en línea recta continuamente
}

