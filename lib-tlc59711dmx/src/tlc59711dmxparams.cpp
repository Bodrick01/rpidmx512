/**
 * @file tlc59711dmxparams.cpp
 *
 */
/* Copyright (C) 2018-2019 by Arjan van Vught mailto:info@raspberrypi-dmx.nl
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdint.h>
#include <string.h>
#ifndef NDEBUG
 #include <stdio.h>
#endif
#include <assert.h>

#ifndef ALIGNED
 #define ALIGNED __attribute__((aligned(4)))
#endif

#include "tlc59711dmxparams.h"
#include "tlc59711dmx.h"

#include "readconfigfile.h"
#include "sscan.h"
#include "propertiesbuilder.h"

#include "devicesparamsconst.h"

#define SET_LED_TYPE_MASK			(1 << 0)
#define SET_LED_COUNT_MASK			(1 << 1)
#define SET_DMX_START_ADDRESS_MASK	(1 << 2)
#define SET_SPI_SPEED_MASK			(1 << 3)

#define TLC59711_TYPES_COUNT 				2
#define TLC59711_TYPES_MAX_NAME_LENGTH 		10
static const char sLedTypes[TLC59711_TYPES_COUNT][TLC59711_TYPES_MAX_NAME_LENGTH] ALIGNED = { "TLC59711\0", "TLC59711W" };

TLC59711DmxParams::TLC59711DmxParams(TLC59711DmxParamsStore *pTLC59711ParamsStore): m_pLC59711ParamsStore(pTLC59711ParamsStore) {
	m_tLC59711Params.nSetList = 0;
	m_tLC59711Params.LedType = TTLC59711_TYPE_RGB;
	m_tLC59711Params.nLedCount = 4;
	m_tLC59711Params.nDmxStartAddress = 1;
	m_tLC59711Params.nSpiSpeedHz = 0;
}

TLC59711DmxParams::~TLC59711DmxParams(void) {
	m_tLC59711Params.nSetList = 0;
}

bool TLC59711DmxParams::Load(void) {
	m_tLC59711Params.nSetList = 0;

	ReadConfigFile configfile(TLC59711DmxParams::staticCallbackFunction, this);

	if (configfile.Read(DevicesParamsConst::DEVICES_TXT)) {
		// There is a configuration file
		if (m_pLC59711ParamsStore != 0) {
			m_pLC59711ParamsStore->Update(&m_tLC59711Params);
		}
	} else if (m_pLC59711ParamsStore != 0) {
		m_pLC59711ParamsStore->Copy(&m_tLC59711Params);
	} else {
		return false;
	}

	return true;
}

void TLC59711DmxParams::Load(const char *pBuffer, uint32_t nLength) {
	assert(pBuffer != 0);
	assert(nLength != 0);
	assert(m_pLC59711ParamsStore != 0);

	m_tLC59711Params.nSetList = 0;

	ReadConfigFile config(TLC59711DmxParams::staticCallbackFunction, this);

	config.Read(pBuffer, nLength);

	m_pLC59711ParamsStore->Update(&m_tLC59711Params);
}

void TLC59711DmxParams::callbackFunction(const char* pLine) {
	assert(pLine != 0);

	uint8_t value8;
	uint16_t value16;
	uint32_t value32;
	uint8_t len;
	char buffer[12];

	len = 9;
	if (Sscan::Char(pLine, DevicesParamsConst::LED_TYPE, buffer, &len) == SSCAN_OK) {
		buffer[len] = '\0';
		if (strcasecmp(buffer, sLedTypes[TTLC59711_TYPE_RGB]) == 0) {
			m_tLC59711Params.LedType = TTLC59711_TYPE_RGB;
			m_tLC59711Params.nSetList |= SET_LED_TYPE_MASK;
		} else if (strcasecmp(buffer, sLedTypes[TTLC59711_TYPE_RGBW]) == 0) {
			m_tLC59711Params.LedType = TTLC59711_TYPE_RGBW;
			m_tLC59711Params.nSetList |= SET_LED_TYPE_MASK;
		}
		return;
	}

	if (Sscan::Uint8(pLine, DevicesParamsConst::LED_COUNT, &value8) == SSCAN_OK) {
		if ((value8 != 0) && (value8 <= 170)) {
			m_tLC59711Params.nLedCount = value8;
			m_tLC59711Params.nSetList |= SET_LED_COUNT_MASK;
		}
		return;
	}

	if (Sscan::Uint16(pLine, DevicesParamsConst::DMX_START_ADDRESS, &value16) == SSCAN_OK) {
		if ((value16 != 0) && (value16 <= 512)) {
			m_tLC59711Params.nDmxStartAddress = value16;
			m_tLC59711Params.nSetList |= SET_DMX_START_ADDRESS_MASK;
		}
		return;
	}

	if (Sscan::Uint32(pLine, DevicesParamsConst::SPI_SPEED_HZ, &value32) == SSCAN_OK) {
		m_tLC59711Params.nSpiSpeedHz = value32;
		m_tLC59711Params.nSetList |= SET_SPI_SPEED_MASK;
	}
}

void TLC59711DmxParams::Set(TLC59711Dmx* pTLC59711Dmx) {
	if (m_tLC59711Params.nSetList == 0) {
		return;
	}

	if(isMaskSet(SET_LED_TYPE_MASK)) {
		pTLC59711Dmx->SetLEDType(m_tLC59711Params.LedType);
	}

	if(isMaskSet(SET_LED_COUNT_MASK)) {
		pTLC59711Dmx->SetLEDCount(m_tLC59711Params.nLedCount);
	}

	if(isMaskSet(SET_DMX_START_ADDRESS_MASK)) {
		pTLC59711Dmx->SetDmxStartAddress(m_tLC59711Params.nDmxStartAddress);
	}

	if(isMaskSet(SET_SPI_SPEED_MASK)) {
		pTLC59711Dmx->SetSpiSpeedHz(m_tLC59711Params.nSpiSpeedHz);
	}
}

void TLC59711DmxParams::Dump(void) {
#ifndef NDEBUG
	if (m_tLC59711Params.nSetList == 0) {
		return;
	}

	printf("%s::%s \'%s\':\n", __FILE__, __FUNCTION__, DevicesParamsConst::DEVICES_TXT);

	if(isMaskSet(SET_LED_TYPE_MASK)) {
		printf(" %s=%s [%d]\n", DevicesParamsConst::LED_TYPE, sLedTypes[m_tLC59711Params.LedType], (int) m_tLC59711Params.LedType);
	}

	if(isMaskSet(SET_LED_COUNT_MASK)) {
		printf(" %s=%d\n", DevicesParamsConst::LED_COUNT, m_tLC59711Params.nLedCount);
	}

	if(isMaskSet(SET_DMX_START_ADDRESS_MASK)) {
		printf(" %s=%d\n", DevicesParamsConst::DMX_START_ADDRESS, m_tLC59711Params.nDmxStartAddress);
	}

	if(isMaskSet(SET_SPI_SPEED_MASK)) {
		printf(" %s=%d Hz\n", DevicesParamsConst::SPI_SPEED_HZ, m_tLC59711Params.nSpiSpeedHz);
	}
#endif
}

const char* TLC59711DmxParams::GetLedTypeString(TTLC59711Type tTTLC59711Type) {
	assert (tTTLC59711Type < TLC59711_TYPES_COUNT);

	return sLedTypes[tTTLC59711Type];
}

bool TLC59711DmxParams::IsSetLedType(void) const {
	return isMaskSet(SET_LED_TYPE_MASK);
}

bool TLC59711DmxParams::IsSetLedCount(void) const {
	return isMaskSet(SET_LED_COUNT_MASK);
}

void TLC59711DmxParams::staticCallbackFunction(void* p, const char* s) {
	assert(p != 0);
	assert(s != 0);

	((TLC59711DmxParams *) p)->callbackFunction(s);
}

bool TLC59711DmxParams::isMaskSet(uint32_t nMask) const {
	return (m_tLC59711Params.nSetList & nMask) == nMask;
}
