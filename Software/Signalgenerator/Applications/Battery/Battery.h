#ifndef BATTERY_BATTERY_H_
#define BATTERY_BATTERY_H_

#include <stdint.h>
#include <string.h>

/*
 *
 *       ___         ___
 *  .---|___|---o---|___|---o--o
 *  |     R1    |     R2    |
 *  |           |    ||     |  |
 * ---          '----||-----'  |
 *  - E              ||       Vbat
 *  |                          |
 *  |                          |
 *  |
 *  '--------------------------o
 *
 */

typedef struct {
	/* Point coordinates */
	/* State-of-Charge in u-percent */
	uint32_t SoC;

	/* Battery characteristics at that point */
	/* Equilibrium potential in uV */
	uint32_t E;
	/* Purely resistive part of internal resistance in uR */
	uint32_t R1;
	/* RC part of internal resistance in uR and uF */
	uint32_t R2;
	uint32_t C;
} BatDataPoint_t;

typedef struct {
	BatDataPoint_t state;
	/* Fully charged capacity [uAh] */
	int32_t capacityFull;
	int32_t capacity;
	/* Voltage of the capacitor in the RC part */
	int32_t CVoltage;
	/* Timestamp of last update */
	uint32_t lastUpdate;
	uint16_t npoints;
	BatDataPoint_t *profile;
} Battery_t;

int8_t Battery_Load(Battery_t * bat, const char *filename);
int8_t Battery_Save(Battery_t * bat, const char *filename);
/**
 * @brief Interpolates battery data to a given SoC
 * @param profile Pointer to the first data point of the profile
 * @param npoints Number of points in the profile
 * @param point Point to interpolate. Point coordinates must be set, function sets the characteristics
 */
void Battery_Interpolate(const BatDataPoint_t *profile, uint16_t npoints,
		BatDataPoint_t *point);


void Battery_Update(Battery_t *b, int32_t current);

void Battery_NewSoc(Battery_t *b, uint32_t SoC);

void Battery_NewCapacity(Battery_t *b, uint32_t capacity);

#endif
