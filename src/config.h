#ifndef CONFIG_H
#define CONFIG_H

#define MOTOR_SPD_FORWARD ( 384 )
#define MOTOR_SPD_OFFSET_L ( +0 )
#define MOTOR_SPD_OFFSET_R ( +0 )
#define MOTOR_SPD_DEAD ( 0 ) // Reverse Threshold

#define FWD (0xAA)
#define LEFT_REV (0x66)
#define RIGHT_REV (0x99)
#define STP (0x00)

#define SENSOR_THRESHOLD ( 512U )

#define INVERTED_PWM ( 1 )

#define PID_KP ( 14.4f )
#define PID_KI ( 1.2f )
#define PID_KD ( 3.6f )

#endif /* CONFIG_H */
