#include "IR_NEC.h"

/* Standalone visual-effect modes.  Variables live in main.c and are
 * changed by the TIM1 NEC IR decoder through KeyHandle(). */
extern volatile uint8_t cubeEffectMode;
extern volatile uint8_t effectBrightnessIndex;
extern volatile uint8_t effectSpeedIndex;
extern volatile uint8_t effectPaused;
extern volatile uint8_t effectPowerEnabled;
extern volatile uint8_t manualRed;
extern volatile uint8_t manualGreen;
extern volatile uint8_t manualBlue;
extern volatile uint8_t manualBrightness;
extern volatile uint8_t diagnosticOrientation;
extern volatile uint8_t diagnosticIndex;
extern volatile uint8_t diagnosticColor;
extern volatile uint8_t diagnosticBrightness;
extern volatile uint8_t diagnosticCount;
extern volatile uint8_t ballHueCycleEnabled;
extern volatile uint8_t ballBounceEnabled;

#define BRIGHTNESS_LEVELS 8U
#define SPEED_LEVELS      9U
#define RGB_STEP          8U
#define MANUAL_BRIGHTNESS_STEP 16U

/* PLAY in manual RGB mode cycles visible primary presets.
 * 0 = next press selects pure red, 1 = green, 2 = blue. */
static uint8_t manualPrimaryPreset = 0U;

static void channel_up(volatile uint8_t *value)
{
    if (*value > (uint8_t)(255U - RGB_STEP)) *value = 255U;
    else *value = (uint8_t)(*value + RGB_STEP);
}

static void channel_down(volatile uint8_t *value)
{
    if (*value < RGB_STEP) *value = 0U;
    else *value = (uint8_t)(*value - RGB_STEP);
}

static void manual_brightness_up(void)
{
    if (manualBrightness > (uint8_t)(255U - MANUAL_BRIGHTNESS_STEP)) manualBrightness = 255U;
    else manualBrightness = (uint8_t)(manualBrightness + MANUAL_BRIGHTNESS_STEP);
}

static void manual_brightness_down(void)
{
    if (manualBrightness < MANUAL_BRIGHTNESS_STEP) manualBrightness = 0U;
    else manualBrightness = (uint8_t)(manualBrightness - MANUAL_BRIGHTNESS_STEP);
}

void KeyHandle(char key)
{
    switch ((uint8_t)key)
    {
        case 0x0c: /* 1: smooth whole-cube hue sweep */
            cubeEffectMode = 0U;
            effectPowerEnabled = 1U;
            break;

        case 0x18: /* 2: vivid cloud swirl */
            cubeEffectMode = 1U;
            effectPowerEnabled = 1U;
            break;

        case 0x5e: /* 3: manual solid RGB */
            cubeEffectMode = 2U;
            effectPowerEnabled = 1U;
            break;

        case 0x44: /* MODE/TEST: cycle hue -> cloud -> manual RGB -> diagnostic -> ball */
            cubeEffectMode++;
            if (cubeEffectMode > 4U) cubeEffectMode = 0U;
            if (cubeEffectMode == 3U)
            {
                diagnosticOrientation = 0U;
                diagnosticIndex = 0U;
                diagnosticColor = 3U;
                diagnosticBrightness = 255U;
                diagnosticCount = 1U;
            }
            if (cubeEffectMode == 4U) effectPaused = 0U;
            effectPowerEnabled = 1U;
            break;

        case 0x40: /* + : overall brightness up */
            if (cubeEffectMode == 3U)
            {
                if (diagnosticBrightness > (uint8_t)(255U - MANUAL_BRIGHTNESS_STEP)) diagnosticBrightness = 255U;
                else diagnosticBrightness = (uint8_t)(diagnosticBrightness + MANUAL_BRIGHTNESS_STEP);
            }
            else if (cubeEffectMode == 2U)
                manual_brightness_up();
            else if (effectBrightnessIndex + 1U < BRIGHTNESS_LEVELS)
                effectBrightnessIndex++;
            break;

        case 0x19: /* - : overall brightness down */
            if (cubeEffectMode == 3U)
            {
                if (diagnosticBrightness < MANUAL_BRIGHTNESS_STEP) diagnosticBrightness = 0U;
                else diagnosticBrightness = (uint8_t)(diagnosticBrightness - MANUAL_BRIGHTNESS_STEP);
            }
            else if (cubeEffectMode == 2U)
                manual_brightness_down();
            else if (effectBrightnessIndex > 0U)
                effectBrightnessIndex--;
            break;

        case 0x08: /* 4: diagnostic add adjacent slice; manual red up */
            if (cubeEffectMode == 3U)
            {
                if (diagnosticCount < 8U) diagnosticCount++;
                /* If the enlarged block would run past slice 8, slide its
                 * start down just enough to keep the whole block visible. */
                if (diagnosticIndex > (uint8_t)(8U - diagnosticCount))
                    diagnosticIndex = (uint8_t)(8U - diagnosticCount);
            }
            else if (cubeEffectMode == 2U || cubeEffectMode == 4U) channel_up(&manualRed);
            break;

        case 0x42: /* 7: diagnostic remove adjacent slice; manual red down */
            if (cubeEffectMode == 3U)
            {
                if (diagnosticCount > 1U) diagnosticCount--;
                /* Keep the current block start when reducing the load. */
                if (diagnosticIndex > (uint8_t)(8U - diagnosticCount))
                    diagnosticIndex = (uint8_t)(8U - diagnosticCount);
            }
            else if (cubeEffectMode == 2U || cubeEffectMode == 4U) channel_down(&manualRed);
            break;

        case 0x1c: /* 5: manual green up */
            if (cubeEffectMode == 2U || cubeEffectMode == 4U) channel_up(&manualGreen);
            break;

        case 0x52: /* 8: manual green down */
            if (cubeEffectMode == 2U || cubeEffectMode == 4U) channel_down(&manualGreen);
            break;

        case 0x5a: /* 6: manual blue up */
            if (cubeEffectMode == 2U || cubeEffectMode == 4U) channel_up(&manualBlue);
            break;

        case 0x4a: /* 9: manual blue down */
            if (cubeEffectMode == 2U || cubeEffectMode == 4U) channel_down(&manualBlue);
            break;

        case 0x09: /* FF: slide diagnostic block upward; otherwise faster animation */
            if (cubeEffectMode == 3U)
            {
                uint8_t maxStart = (uint8_t)(8U - diagnosticCount);
                if (diagnosticIndex < maxStart) diagnosticIndex++;
                else diagnosticIndex = 0U;
            }
            else if (cubeEffectMode != 2U && effectSpeedIndex + 1U < SPEED_LEVELS)
                effectSpeedIndex++;
            break;

        case 0x07: /* REW: slide diagnostic block downward; otherwise slower animation */
            if (cubeEffectMode == 3U)
            {
                uint8_t maxStart = (uint8_t)(8U - diagnosticCount);
                if (diagnosticIndex > 0U) diagnosticIndex--;
                else diagnosticIndex = maxStart;
            }
            else if (cubeEffectMode != 2U && effectSpeedIndex > 0U)
                effectSpeedIndex--;
            break;

        case 0x15: /* PLAY: diagnostic R->G->B->W; manual presets; animation pause */
            if (cubeEffectMode == 3U)
            {
                diagnosticColor = (uint8_t)((diagnosticColor + 1U) & 3U);
            }
            else if (cubeEffectMode == 4U)
            {
                ballBounceEnabled ^= 1U;
            }
            else if (cubeEffectMode == 2U)
            {
                if (manualPrimaryPreset == 0U)
                {
                    manualRed = 255U;
                    manualGreen = 0U;
                    manualBlue = 0U;
                    manualPrimaryPreset = 1U;
                }
                else if (manualPrimaryPreset == 1U)
                {
                    manualRed = 0U;
                    manualGreen = 255U;
                    manualBlue = 0U;
                    manualPrimaryPreset = 2U;
                }
                else
                {
                    manualRed = 0U;
                    manualGreen = 0U;
                    manualBlue = 255U;
                    manualPrimaryPreset = 0U;
                }
            }
            else
                effectPaused ^= 1U;
            break;

        case 0x45: /* POWER: display on/off, retaining settings */
            effectPowerEnabled ^= 1U;
            break;

        case 0x16: /* 0: diagnostic Layer/Row toggle; otherwise reset defaults */
            if (cubeEffectMode == 3U)
            {
                diagnosticOrientation ^= 1U;
                diagnosticIndex = 0U;
                diagnosticCount = 1U;
            }
            else if (cubeEffectMode == 4U)
            {
                ballHueCycleEnabled ^= 1U;
            }
            else
            {
                cubeEffectMode = 1U;
                effectBrightnessIndex = 4U; /* animated effects: 25% */
                effectSpeedIndex = 4U;      /* 100% speed */
                effectPaused = 0U;
                effectPowerEnabled = 1U;
                manualRed = 255U;
                manualGreen = 255U;
                manualBlue = 255U;
                manualBrightness = 64U;     /* manual mode starts at safe 25%, but can reach 255 */
                manualPrimaryPreset = 0U;   /* next PLAY selects pure red */
                diagnosticOrientation = 0U;
                diagnosticIndex = 0U;
                diagnosticColor = 3U;
                diagnosticBrightness = 255U;
                diagnosticCount = 1U;
                ballHueCycleEnabled = 1U;
                ballBounceEnabled = 0U;
            }
            break;

        default:
            break;
    }
}

/* This decoder currently reports completed NEC key frames through KeyHandle(). */
void KeyLongPressHandle(char key)
{
    (void)key;
}
