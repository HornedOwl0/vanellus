#ifndef CONFIG_H
#define CONFIG_H

#define MOTOR_SPD_FORWARD ( 624 )
#define MOTOR_SPD_OFFSET_L ( +0 )
#define MOTOR_SPD_OFFSET_R ( +0 )
#define MOTOR_SPD_DEAD ( 0 ) // Reverse Threshold

#define FWD (0x0A)
#define LEFT_REV (0x06)
#define RIGHT_REV (0x09)
#define STP (0x00)

#define SENSOR_THRESHOLD ( 512U )

#endif /* CONFIG_H */