// Pines ultrasonic
#define TRIG_PIN 13  
#define ECHO_PIN 12 

int calculate_dist() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(3);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);


  // Leo el tiempo que tarda el pulso en regresar
  // y calculo la distancia (en cm)

  float dist = (pulseIn(ECHO_PIN, HIGH) / 2) * 0.0343; // 0.0343 cm/microsegundo
  return dist;
}


void setup() {
  // Iniciar la comunicación serial a 9600 baudios
  Serial.begin(9600);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
}

void loop() {
  Serial.println(calculate_dist());
  delay(1000);
}

