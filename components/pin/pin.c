#include <stdio.h>
#include "soc/reg_base.h" // DR_REG_GPIO_BASE, DR_REG_IO_MUX_BASE
#include "driver/rtc_io.h" // rtc_gpio_*
#include "pin.h"


#define GPIO_OUT_REG                   (DR_REG_GPIO_BASE+0x04)
#define GPIO_OUT1_REG                  (DR_REG_GPIO_BASE+0x10)
#define GPIO_ENABLE_REG                (DR_REG_GPIO_BASE+0x20)
#define GPIO_ENABLE1_REG               (DR_REG_GPIO_BASE+0x2C)
#define GPIO_IN_REG                    (DR_REG_GPIO_BASE+0x3C)
#define GPIO_IN1_REG                   (DR_REG_GPIO_BASE+0x40)
#define GPIO_PIN_REG_BASE              (DR_REG_GPIO_BASE+0x88)
#define GPIO_PIN_REG(N)                (GPIO_PIN_REG_BASE+N*0x04)
#define GPIO_FUNC_IN_SEL_CFG_REG_BASE  (DR_REG_GPIO_BASE+0x130)
#define GPIO_FUNC_IN_SEL_CFG_REG(N)    (GPIO_FUNC_IN_SEL_CFG_REG_BASE+N*0x04)
#define GPIO_FUNC_OUT_SEL_CFG_REG_BASE (DR_REG_GPIO_BASE+0x530)
#define GPIO_FUNC_OUT_SEL_CFG_REG(N)   (GPIO_FUNC_OUT_SEL_CFG_REG_BASE+N*0x04)

#define GPIO_PIN_PAD_DRIVER 2
 
#define IO_MUX_REG(N)                  (DR_REG_IO_MUX_BASE+PIN_MUX_REG_OFFSET[N])

#define FUN_WPD  7
#define FUN_WPU 8
#define FUN_IE 9

#define REG(R) (*(volatile uint32_t *)(R))
#define REG_BITS 32
#define REG_SET_BIT(R, B) (REG(R) |= (0x01 << (B)))
#define REG_CLR_BIT(R, B) (REG(R) &= ~(0x01 << (B)))
#define REG_GET_BIT(R, B) ((REG(R) >> B) & 0x01)

#define GPIO_PIN_REG_DEFAULT 0x0
#define GPIO_FUNC_OUT_SEL_DEFAULT 0x100
#define IO_MUX_REG_DEFAULT 0x2900

// Gives byte offset of IO_MUX Configuration Register
// from base address DR_REG_IO_MUX_BASE
static const uint8_t PIN_MUX_REG_OFFSET[] = {
    0x44, 0x88, 0x40, 0x84, 0x48, 0x6c, 0x60, 0x64, // pin  0- 7
    0x68, 0x54, 0x58, 0x5c, 0x34, 0x38, 0x30, 0x3c, // pin  8-15
    0x4c, 0x50, 0x70, 0x74, 0x78, 0x7c, 0x80, 0x8c, // pin 16-23
    0x90, 0x24, 0x28, 0x2c, 0xFF, 0xFF, 0xFF, 0xFF, // pin 24-31
    0x1c, 0x20, 0x14, 0x18, 0x04, 0x08, 0x0c, 0x10, // pin 32-39
};


// Reset the configuration of a pin to not be an input or an output.
// Pull-up is enabled so the pin does not float.
int32_t pin_reset(pin_num_t pin)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		rtc_gpio_deinit(pin);
		rtc_gpio_pullup_en(pin);
		rtc_gpio_pulldown_dis(pin);
	}
	// TODO: Reset GPIO_PINn_REG: All fields zero
	REG(GPIO_PIN_REG(pin)) = GPIO_PIN_REG_DEFAULT;
	// TODO: Reset GPIO_FUNCn_OUT_SEL_CFG_REG: GPIO_FUNCn_OUT_SEL=0x100
	REG(GPIO_FUNC_OUT_SEL_CFG_REG(pin)) = GPIO_FUNC_OUT_SEL_DEFAULT;
	// TODO: Reset IO_MUX_x_REG: MCU_SEL=2, FUN_DRV=2, FUN_WPU=1
	REG(IO_MUX_REG(pin)) = IO_MUX_REG_DEFAULT;
	// NOTE: By default, pin should not float, save power with FUN_WPU=1
	// Now that the pin is reset, set the output level to zero
	return pin_set_level(pin, 0);
}

// Enable or disable a pull-up on the pin.
int32_t pin_pullup(pin_num_t pin, bool enable)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pullup_en(pin);
		else return rtc_gpio_pullup_dis(pin);
	}
	if (enable) {
		return REG_SET_BIT(IO_MUX_REG(pin), FUN_WPU);
	}
	else {
		return REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPU);
	}
}

// Enable or disable a pull-down on the pin.
int32_t pin_pulldown(pin_num_t pin, bool enable)
{
	if (rtc_gpio_is_valid_gpio(pin)) { // hand-off work to RTC subsystem
		if (enable) return rtc_gpio_pulldown_en(pin);
		else return rtc_gpio_pulldown_dis(pin);
	}
	if (enable) {
		return REG_SET_BIT(IO_MUX_REG(pin), FUN_WPD);
	}
	else {
		return REG_CLR_BIT(IO_MUX_REG(pin), FUN_WPD);

	}
}

// Enable or disable the pin as an input signal.
int32_t pin_input(pin_num_t pin, bool enable)
{
	if (enable) {
		return REG_SET_BIT(IO_MUX_REG(pin), FUN_IE);
	}
	else {
		return REG_CLR_BIT(IO_MUX_REG(pin), FUN_IE);

	}
}

// Enable or disable the pin as an output signal.
int32_t pin_output(pin_num_t pin, bool enable)
{

	if (pin < REG_BITS) {		
		if (enable) {
			return REG_SET_BIT(GPIO_ENABLE_REG, pin);
		}
		else {
			return REG_CLR_BIT(GPIO_ENABLE_REG, pin);
		}
	}	
	else {
		if (enable) {
			return REG_SET_BIT(GPIO_ENABLE1_REG, (pin - REG_BITS));
		}
		else {
			return REG_CLR_BIT(GPIO_ENABLE1_REG, (pin - REG_BITS));
		}
	}
}

// Enable or disable the pin as an open-drain signal.
int32_t pin_odrain(pin_num_t pin, bool enable)
{
	if (enable) {
		return REG_SET_BIT(GPIO_PIN_REG(pin), GPIO_PIN_PAD_DRIVER);
	}
	else {
		return REG_CLR_BIT(GPIO_PIN_REG(pin), GPIO_PIN_PAD_DRIVER);
	}
}

// Sets the output signal level if the pin is configured as an output.
int32_t pin_set_level(pin_num_t pin, int32_t level)
{
	if (pin < REG_BITS)
		if (level > 0) {
			return REG_SET_BIT(GPIO_OUT_REG, pin);
		}
		else {
			return REG_CLR_BIT(GPIO_OUT_REG, pin);
		}
	else {
		if (level > 0) {
			return REG_SET_BIT(GPIO_OUT1_REG, (pin - REG_BITS));
		}
		else {
			return REG_CLR_BIT(GPIO_OUT1_REG, (pin - REG_BITS));
		}
	}
	
}

// Gets the input signal level if the pin is configured as an input.
int32_t pin_get_level(pin_num_t pin)
{
	if (pin < REG_BITS)
		return REG_GET_BIT(GPIO_IN_REG, pin);

	else {
		return REG_GET_BIT(GPIO_IN1_REG, (pin - REG_BITS));
	}
}

// Get the value of the input registers, one pin per bit.
// The two 32-bit input registers are concatenated into a uint64_t.
uint64_t pin_get_in_reg(void)
{
    uint64_t outReg1 = REG(GPIO_OUT_REG);
    uint64_t outReg2 = REG(GPIO_OUT1_REG);
    return (outReg2 << REG_BITS) | outReg1;
}

// Get the value of the output registers, one pin per bit.
// The two 32-bit output registers are concatenated into a uint64_t.
uint64_t pin_get_out_reg(void)
{
    uint64_t outReg1 = REG(GPIO_OUT_REG);
    uint64_t outReg2 = REG(GPIO_OUT1_REG);
    return (outReg2 << REG_BITS) | outReg1;
}
