#include <Arduino.h>

#define DELAY_TIM 10 // 5だとチャタリングする

#define chmax(a, b) ((a) = max((a), (b)))
#define chmin(a, b) ((a) = min((a), (b)))

#define DP_7SEG PORTB3
#define G_7SEG PORTB2
#define F_7SEG PORTB1
#define E_7SEG PORTB0

#define D_7SEG PORTD7
#define C_7SEG PORTD6
#define B_7SEG PORTD5
#define A_7SEG PORTD4

#define __SW1 (~PINC & (1 << PINC2))
#define __SW2 (~PINC & (1 << PINC1))
#define __SW3 (~PIND & (1 << PIND2))
#define __SW4 (~PIND & (1 << PIND3))

#define blank 11
#define minus 12

const unsigned char data[][8] = {
    // DP, g, f, e, d, c, b, a
    {0, 0, 1, 1, 1, 1, 1, 1}, // 0
    {0, 0, 0, 0, 0, 1, 1, 0}, // 1
    {0, 1, 0, 1, 1, 0, 1, 1}, // 2
    {0, 1, 0, 0, 1, 1, 1, 1}, // 3
    {0, 1, 1, 0, 0, 1, 1, 0}, // 4
    {0, 1, 1, 0, 1, 1, 0, 1}, // 5
    {0, 1, 1, 1, 1, 1, 0, 1}, // 6
    {0, 0, 1, 0, 0, 1, 1, 1}, // 7
    {0, 1, 1, 1, 1, 1, 1, 1}, // 8
    {0, 1, 1, 0, 1, 1, 1, 1}, // 9
    {0, 1, 0, 1, 1, 1, 1, 0}, // d
    {0, 0, 0, 0, 0, 0, 0, 0}, // blank
    {0, 1, 0, 0, 0, 0, 0, 0}  // -
};

const unsigned char resiter_data[8]{
    DP_7SEG, G_7SEG, F_7SEG, E_7SEG, D_7SEG, C_7SEG, B_7SEG, A_7SEG};

void numprint(const int num)
{
  for (int i = 0; i < 4; i++)
  {
    if (data[num][i])
      PORTB |= (1 << resiter_data[i]);
    else
      PORTB &= ~(1 << resiter_data[i]);
  }
  for (int i = 4; i < 8; i++)
  {
    if (data[num][i])
      PORTD |= (1 << resiter_data[i]);
    else
      PORTD &= ~(1 << resiter_data[i]);
  }
}

void toggleTransistor(const int digit, const int toggle)
{
  if (digit == 10)
  {
    if (toggle)
    {
      PORTB |= (1 << PORTB5);
    }
    else
    {
      PORTB &= ~(1 << PORTB5);
    }
  }
  else
  {
    if (toggle)
    {
      PORTB |= (1 << PORTB4);
    }
    else
    {
      PORTB &= ~(1 << PORTB4);
    }
  }
}

void numprint(const int num, const int digit)
{
  numprint(num);
  if (digit == 1)
  {
    toggleTransistor(1, 1);
    toggleTransistor(10, 0);
  }
  else
  {
    toggleTransistor(1, 0);
    toggleTransistor(10, 1);
  }
}

ISR(INT0_vect)
{
  EIMSK &= ~0b00000011;

  Serial.write(0x00);

  int cnt = 0;
  while (1)
  {
    if (!__SW3)
    {
      if (cnt <= 20)
      {
        cnt++;
      }
      else
      {
        break;
      }
    }
    else
    {
      cnt = 0;
    }
  }

  EIMSK |= (1 << INT0) | (1 << INT1);
}

ISR(INT1_vect)
{
  EIMSK &= ~0b00000011;
  int cnt = 0;

  while (1)
  {
    if (!__SW4)
    {
      if (cnt <= 20)
      {
        cnt++;
      }
      else
      {
        break;
      }
    }
    else
    {
      cnt = 0;
    }
  }

  Serial.write(0x01);

  EIMSK |= (1 << INT0) | (1 << INT1);
}

void setup()
{
  DDRD = 0b11110000;
  DDRB = 0b00111111;
  EIMSK |= (1 << INT0);
  EICRA &= ~((1 << ISC01) | (1 << ISC00));
  EIMSK |= (1 << INT1);
  EICRA &= ~((1 << ISC11) | (1 << ISC10));
  sei();
  Serial.begin(115200);
  Serial.write(0x02);
}

void loop()
{
  static int speed = 12;
  if (Serial.available())
  {
    speed = Serial.read();
    Serial.write(0x02);
  }
  numprint(speed / 10, 10);
  delay(DELAY_TIM);
  numprint(speed % 10, 1);
  delay(DELAY_TIM);
}