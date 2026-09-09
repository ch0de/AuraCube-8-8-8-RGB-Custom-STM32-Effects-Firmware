#include "System.h"

uint16_t strCount = 0;

/* ===== clock / utils (unchanged) ===== */
void SystemClock(uint32_t pllMull)
{
    __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

    RCC_DeInit();
    RCC->CR |= ((uint32_t)RCC_CR_HSEON);
    do { HSEStatus = RCC->CR & RCC_CR_HSERDY; StartUpCounter++; }
    while((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

    if ((RCC->CR & RCC_CR_HSERDY) != RESET) HSEStatus = (uint32_t)0x01; else HSEStatus = (uint32_t)0x00;

    if (HSEStatus == (uint32_t)0x01)
    {
        FLASH->ACR |= FLASH_ACR_PRFTBE;
        FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
        FLASH->ACR |= (uint32_t)FLASH_ACR_LATENCY_2;

        RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;
        RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;
        RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV2;
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLMULL));
        RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLSRC_HSE | pllMull);

        RCC->CR |= RCC_CR_PLLON;
        while((RCC->CR & RCC_CR_PLLRDY) == 0) { }
        RCC->CFGR &= (uint32_t)((uint32_t)~(RCC_CFGR_SW));
        RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)0x08) { }
    }
}

void ID_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < SystemCoreClock / (300 * 1000 / ms); i++) { }
}

void JTAGDisable(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_PinRemapConfig (GPIO_Remap_SWJ_Disable, ENABLE);
    GPIO_PinRemapConfig (GPIO_Remap_SWJ_JTAGDisable, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3 | GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_Init (GPIOB, &GPIO_InitStructure);
    RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO, ENABLE);
}

void JTAGEnable(void)
{
    GPIO_PinRemapConfig (GPIO_Remap_SWJ_Disable, DISABLE);
    GPIO_PinRemapConfig (GPIO_Remap_SWJ_JTAGDisable, DISABLE);
}

void sysTickInit(void)
{
    if (SysTick_Config(SystemCoreClock / 1000)) { while(1); }
    NVIC_SetPriority(SysTick_IRQn, 0);
}

/* ===== Dual smooth effects + IR remote control =====
 *
 * Modes:
 *   0 = whole-cube continuous hue sweep (the smooth v4 diagnostic effect)
 *   1 = vivid 3-D cloud swirl
 *   2 = manual solid RGB color
 *   3 = slice diagnostic (layers / rows, R/G/B/W)
 *   4 = breathing / bouncing anti-aliased ball
 *
 * Remote controls are handled in Common/USER/src/IR.c:
 *   1           = solid hue sweep
 *   2           = cloud swirl
 *   3           = manual solid RGB
 *   MODE/TEST   = cycle hue -> cloud -> manual RGB -> diagnostic -> breathing ball
 *   + / -       = brightness up/down
 *   manual RGB: 4/7 red up/down, 5/8 green up/down, 6/9 blue up/down
 *   FF / REW    = speed up/down in animated modes
 *   PLAY        = pause/resume in animations; in manual RGB cycle pure R -> G -> B
 *   diagnostic: PLAY cycles R/G/B/W; 0 toggles layer/row
 *   breathing ball: PLAY toggles bounce; 0 toggles solid color / smooth hue rotation
 *               4 adds cumulative slices 1->2->...->8; 7 removes them
 *               FF/REW slide the entire active slice block up/down
 *   POWER       = display on/off
 *   0           = reset controls to defaults
 *
 * Default brightness is exactly the proven v4 25% electrical baseline.
 * Brightness can be adjusted from about 6% to 44%; the upper levels are
 * intentionally capped below the old 100% condition that produced buzzing
 * and layer flicker on USB power.
 */
#define EFFECT_FRAME_MS 16U
#define VOXEL_COUNT     512U
#define RGB_Q8_FULL     (255U << 8)

volatile uint8_t cubeEffectMode = 1U;          /* 0 hue, 1 cloud, 2 manual RGB, 3 diagnostic, 4 ball */
volatile uint8_t effectBrightnessIndex = 4U;   /* 25% default for animated effects */
volatile uint8_t effectSpeedIndex = 4U;        /* 100% default */
volatile uint8_t effectPaused = 0U;
volatile uint8_t effectPowerEnabled = 1U;

/* Manual solid RGB mode.  RGB channels are independently adjustable over the
 * full 0..255 range.  Overall manual brightness is also 0..255 and, unlike
 * the animated modes, is intentionally NOT capped.  It starts at the same
 * safe 25% level used by the animations; + can raise it all the way to 100%. */
volatile uint8_t manualRed = 255U;
volatile uint8_t manualGreen = 255U;
volatile uint8_t manualBlue = 255U;
volatile uint8_t manualBrightness = 64U;

/* Slice diagnostic mode.  Physical cube layers are Y=0..7.  "Rows" are
 * vertical Z slices, giving eight 8x8 planes through the cube.
 * Starts at layer 1, white, full brightness to make marginal layer drive
 * behavior easy to see; +/- can back the brightness down. */
volatile uint8_t diagnosticOrientation = 0U;  /* 0 = horizontal layer (Y), 1 = vertical row plane (Z) */
volatile uint8_t diagnosticIndex = 0U;        /* 0..7; start of active block */
volatile uint8_t diagnosticColor = 3U;        /* 0=R, 1=G, 2=B, 3=W */
volatile uint8_t diagnosticBrightness = 255U;
volatile uint8_t diagnosticCount = 1U;        /* number of adjacent active layers/rows */


/* Breathing ball mode.  It begins centered and smoothly expands from a small
 * sphere to fill the cube, then contracts.  The sphere edge is anti-aliased
 * so voxels fade in/out rather than popping on discrete radius steps.
 *
 * Color can either use the manual RGB values above or continuously rotate
 * through full-saturation hues.  PLAY toggles motion of the center so the
 * breathing sphere can bounce around the cube. */
volatile uint8_t ballHueCycleEnabled = 1U;
volatile uint8_t ballBounceEnabled = 0U;

/* Q0.8 brightness multipliers.  64 = 25%. */
static const uint8_t brightnessScaleTable[] = {
    16U, 24U, 32U, 48U, 64U, 80U, 96U, 112U
};
#define BRIGHTNESS_LEVEL_COUNT ((uint8_t)(sizeof(brightnessScaleTable) / sizeof(brightnessScaleTable[0])))

/* Percent of nominal animation speed. */
static const uint16_t speedPercentTable[] = {
    25U, 40U, 60U, 80U, 100U, 125U, 150U, 200U, 300U
};
#define SPEED_LEVEL_COUNT ((uint8_t)(sizeof(speedPercentTable) / sizeof(speedPercentTable[0])))

static const int8_t sin8_table[256] = {
       0,    3,    6,    9,   12,   16,   19,   22,   25,   28,   31,   34,   37,   40,   43,   46,
      49,   51,   54,   57,   60,   63,   65,   68,   71,   73,   76,   78,   81,   83,   85,   88,
      90,   92,   94,   96,   98,  100,  102,  104,  106,  107,  109,  111,  112,  113,  115,  116,
     117,  118,  120,  121,  122,  122,  123,  124,  125,  125,  126,  126,  126,  127,  127,  127,
     127,  127,  127,  127,  126,  126,  126,  125,  125,  124,  123,  122,  122,  121,  120,  118,
     117,  116,  115,  113,  112,  111,  109,  107,  106,  104,  102,  100,   98,   96,   94,   92,
      90,   88,   85,   83,   81,   78,   76,   73,   71,   68,   65,   63,   60,   57,   54,   51,
      49,   46,   43,   40,   37,   34,   31,   28,   25,   22,   19,   16,   12,    9,    6,    3,
       0,   -3,   -6,   -9,  -12,  -16,  -19,  -22,  -25,  -28,  -31,  -34,  -37,  -40,  -43,  -46,
     -49,  -51,  -54,  -57,  -60,  -63,  -65,  -68,  -71,  -73,  -76,  -78,  -81,  -83,  -85,  -88,
     -90,  -92,  -94,  -96,  -98, -100, -102, -104, -106, -107, -109, -111, -112, -113, -115, -116,
    -117, -118, -120, -121, -122, -122, -123, -124, -125, -125, -126, -126, -126, -127, -127, -127,
    -127, -127, -127, -127, -126, -126, -126, -125, -125, -124, -123, -122, -122, -121, -120, -118,
    -117, -116, -115, -113, -112, -111, -109, -107, -106, -104, -102, -100,  -98,  -96,  -94,  -92,
     -90,  -88,  -85,  -83,  -81,  -78,  -76,  -73,  -71,  -68,  -65,  -63,  -60,  -57,  -54,  -51,
     -49,  -46,  -43,  -40,  -37,  -34,  -31,  -28,  -25,  -22,  -19,  -16,  -12,   -9,   -6,   -3,
};

static uint8_t ditherR[VOXEL_COUNT];
static uint8_t ditherG[VOXEL_COUNT];
static uint8_t ditherB[VOXEL_COUNT];
static uint16_t solidDitherR = 0U;
static uint16_t solidDitherG = 0U;
static uint16_t solidDitherB = 0U;

static int16_t sin16_q8(uint16_t phase)
{
    uint8_t index = (uint8_t)(phase >> 8);
    uint8_t frac  = (uint8_t)phase;
    int16_t a = sin8_table[index];
    int16_t b = sin8_table[(uint8_t)(index + 1U)];
    return (int16_t)((a * 256) + ((b - a) * frac));
}

static uint8_t dither_q8_to_u8(uint16_t value, uint8_t *error)
{
    uint8_t whole = (uint8_t)(value >> 8);
    uint16_t sum = (uint16_t)(*error + (uint8_t)value);
    if (sum >= 256U)
    {
        sum -= 256U;
        if (whole < 255U) whole++;
    }
    *error = (uint8_t)sum;
    return whole;
}

static uint8_t dither_q8_to_u8_shared(uint16_t value, uint16_t *error)
{
    uint8_t whole = (uint8_t)(value >> 8);
    uint16_t sum = (uint16_t)(*error + (value & 0x00FFU));
    if (sum >= 256U)
    {
        sum -= 256U;
        if (whole < 255U) whole++;
    }
    *error = sum;
    return whole;
}

static uint16_t apply_brightness_q8(uint16_t value)
{
    uint8_t idx = effectBrightnessIndex;
    if (idx >= BRIGHTNESS_LEVEL_COUNT) idx = (uint8_t)(BRIGHTNESS_LEVEL_COUNT - 1U);
    return (uint16_t)(((uint32_t)value * brightnessScaleTable[idx]) >> 8);
}

/* Full-saturation HSV -> RGB.  This is intentionally vivid: unlike cloud v1,
 * saturation is always 255, so colors can reach chromatically pure red,
 * green, and blue (subject only to the selected overall brightness limit). */
static void hue16_to_rgb_q8(uint16_t hue, uint16_t value,
                            uint16_t *r, uint16_t *g, uint16_t *b)
{
    uint32_t scaled = (uint32_t)hue * 6U;
    uint8_t sector = (uint8_t)(scaled >> 16);
    uint16_t frac = (uint16_t)scaled;
    uint16_t rise = (uint16_t)(((uint32_t)value * frac) >> 16);
    uint16_t fall = (uint16_t)(value - rise);

    switch (sector)
    {
        default:
        case 0: *r = value; *g = rise;  *b = 0U;    break;
        case 1: *r = fall;  *g = value; *b = 0U;    break;
        case 2: *r = 0U;    *g = value; *b = rise;  break;
        case 3: *r = 0U;    *g = fall;  *b = value; break;
        case 4: *r = rise;  *g = 0U;    *b = value; break;
        case 5: *r = value; *g = 0U;    *b = fall;  break;
    }
}

static void render_solid_hue_frame(uint8_t buffer, uint16_t hue)
{
    uint16_t rq8, gq8, bq8;
    uint8_t r, g, b;

    hue16_to_rgb_q8(hue, RGB_Q8_FULL, &rq8, &gq8, &bq8);
    rq8 = apply_brightness_q8(rq8);
    gq8 = apply_brightness_q8(gq8);
    bq8 = apply_brightness_q8(bq8);

    r = dither_q8_to_u8_shared(rq8, &solidDitherR);
    g = dither_q8_to_u8_shared(gq8, &solidDitherG);
    b = dither_q8_to_u8_shared(bq8, &solidDitherB);

    setLED = buffer;
    setAll(r, g, b);
}

/* Manual RGB mode uses the complete 8-bit channel range and an independent
 * uncapped 0..255 brightness multiplier.  Keeping the multiply in Q8 lets the
 * existing temporal dithering preserve fractional brightness between steps. */
static void render_manual_rgb_frame(uint8_t buffer)
{
    uint16_t rq8, gq8, bq8;
    uint8_t r, g, b;
    uint8_t brightness = manualBrightness;

    rq8 = (uint16_t)(((uint32_t)manualRed   * brightness * 256UL + 127UL) / 255UL);
    gq8 = (uint16_t)(((uint32_t)manualGreen * brightness * 256UL + 127UL) / 255UL);
    bq8 = (uint16_t)(((uint32_t)manualBlue  * brightness * 256UL + 127UL) / 255UL);

    if (rq8 > RGB_Q8_FULL) rq8 = RGB_Q8_FULL;
    if (gq8 > RGB_Q8_FULL) gq8 = RGB_Q8_FULL;
    if (bq8 > RGB_Q8_FULL) bq8 = RGB_Q8_FULL;

    r = dither_q8_to_u8_shared(rq8, &solidDitherR);
    g = dither_q8_to_u8_shared(gq8, &solidDitherG);
    b = dither_q8_to_u8_shared(bq8, &solidDitherB);

    setLED = buffer;
    setAll(r, g, b);
}

static void render_diagnostic_frame(uint8_t buffer)
{
    uint8_t x, y, z;
    uint8_t r = 0U, g = 0U, b = 0U;
    uint8_t level = diagnosticBrightness;
    uint8_t index = (uint8_t)(diagnosticIndex & 7U);
    uint8_t count = diagnosticCount;
    uint8_t end;
    if (count < 1U) count = 1U;
    if (count > 8U) count = 8U;
    /* diagnosticIndex is the first active slice. Keep the complete adjacent
     * block inside the 8x8x8 cube. */
    if (index > (uint8_t)(8U - count)) index = (uint8_t)(8U - count);
    end = (uint8_t)(index + count);

    switch (diagnosticColor & 3U)
    {
        case 0U: r = level; break;
        case 1U: g = level; break;
        case 2U: b = level; break;
        default: r = level; g = level; b = level; break;
    }

    setLED = buffer;
    setAll(0U, 0U, 0U);

    if (diagnosticOrientation == 0U)
    {
        /* Light an adjacent horizontal block: e.g. count=3,index=2 gives
         * physical layers 3+4+5. */
        for (y = index; y < end; y++)
            for (x = 0U; x < 8U; x++)
                for (z = 0U; z < 8U; z++)
                    set(x, y, z, r, g, b);
    }
    else
    {
        /* Same sliding adjacent block for vertical row planes.  The physical
         * row direction on this Aura cube is opposite the internal Z axis,
         * so reverse Z here: diagnostic Row 1 now lights physical Row 1,
         * Row 2 lights physical Row 2, etc. */
        uint8_t logicalRow;
        for (logicalRow = index; logicalRow < end; logicalRow++)
        {
            z = (uint8_t)(7U - logicalRow);
            for (y = 0U; y < 8U; y++)
                for (x = 0U; x < 8U; x++)
                    set(x, y, z, r, g, b);
        }
    }
}

/* Filled 3-D breathing ball with a soft one-voxel edge.  Coordinates and
 * distances are fixed-point only; there are no floating-point operations.
 * center*Q8 uses voxel coordinates 0..7 in Q8.8.  radiusQ8 uses the same
 * units. */
static void render_ball_frame(uint8_t buffer, uint16_t breathPhase, uint16_t hue,
                              int16_t centerXQ8, int16_t centerYQ8, int16_t centerZQ8)
{
    uint8_t x, y, z;
    int32_t wave;
    uint32_t breath;
    uint16_t radiusQ8;
    uint16_t edgeQ8 = 150U;           /* about 0.59 voxel soft edge */
    uint16_t innerRadiusQ8, outerRadiusQ8;
    uint32_t inner2, outer2, edgeSpan;
    uint16_t baseRQ8, baseGQ8, baseBQ8;

    /* sin16_q8 ranges approximately -32768..+32767.  Shift to 0..65535
     * so expansion and contraction ease smoothly at both endpoints. */
    wave = (int32_t)sin16_q8(breathPhase);
    breath = (uint32_t)(wave + 32768L);
    if (breath > 65535UL) breath = 65535UL;

    /* Radius 1.10 -> 6.65 voxels.  At the centered maximum even the eight
     * corners are fully inside the sphere, so the cube genuinely fills. */
    radiusQ8 = (uint16_t)(282U + (((uint32_t)(1702U - 282U) * breath) >> 16));

    if (radiusQ8 > edgeQ8) innerRadiusQ8 = (uint16_t)(radiusQ8 - edgeQ8);
    else innerRadiusQ8 = 0U;
    outerRadiusQ8 = (uint16_t)(radiusQ8 + edgeQ8);
    inner2 = (uint32_t)innerRadiusQ8 * innerRadiusQ8;
    outer2 = (uint32_t)outerRadiusQ8 * outerRadiusQ8;
    edgeSpan = outer2 - inner2;
    if (edgeSpan == 0U) edgeSpan = 1U;

    if (ballHueCycleEnabled)
    {
        hue16_to_rgb_q8(hue, RGB_Q8_FULL, &baseRQ8, &baseGQ8, &baseBQ8);
    }
    else
    {
        baseRQ8 = (uint16_t)((uint16_t)manualRed << 8);
        baseGQ8 = (uint16_t)((uint16_t)manualGreen << 8);
        baseBQ8 = (uint16_t)((uint16_t)manualBlue << 8);
    }

    baseRQ8 = apply_brightness_q8(baseRQ8);
    baseGQ8 = apply_brightness_q8(baseGQ8);
    baseBQ8 = apply_brightness_q8(baseBQ8);

    setLED = buffer;
    setAdd = 0;

    {
        uint16_t voxel = 0U;
        for (y = 0U; y < 8U; y++)
        {
            int32_t dy = ((int32_t)y << 8) - centerYQ8;
            uint32_t dy2 = (uint32_t)(dy * dy);
            for (z = 0U; z < 8U; z++)
            {
                int32_t dz = ((int32_t)z << 8) - centerZQ8;
                uint32_t yz2 = dy2 + (uint32_t)(dz * dz);
                for (x = 0U; x < 8U; x++, voxel++)
                {
                    int32_t dx = ((int32_t)x << 8) - centerXQ8;
                    uint32_t dist2 = yz2 + (uint32_t)(dx * dx);
                    uint16_t coverage;
                    uint16_t rq8, gq8, bq8;
                    uint8_t r, g, b;

                    if (dist2 <= inner2) coverage = 255U;
                    else if (dist2 >= outer2) coverage = 0U;
                    else coverage = (uint16_t)(((outer2 - dist2) * 255UL) / edgeSpan);

                    rq8 = (uint16_t)(((uint32_t)baseRQ8 * coverage + 127UL) / 255UL);
                    gq8 = (uint16_t)(((uint32_t)baseGQ8 * coverage + 127UL) / 255UL);
                    bq8 = (uint16_t)(((uint32_t)baseBQ8 * coverage + 127UL) / 255UL);

                    r = dither_q8_to_u8(rq8, &ditherR[voxel]);
                    g = dither_q8_to_u8(gq8, &ditherG[voxel]);
                    b = dither_q8_to_u8(bq8, &ditherB[voxel]);
                    set(x, y, z, r, g, b);
                }
            }
        }
    }
}

static void render_cloud_frame(uint8_t buffer,
                               uint16_t phase1, uint16_t phase2,
                               uint16_t phase3, uint16_t phase4,
                               uint16_t huePhase)
{
    uint16_t voxel = 0;
    uint8_t x, y, z;

    setLED = buffer;
    setAdd = 0;

    for (y = 0; y < 8U; y++)
    {
        for (z = 0; z < 8U; z++)
        {
            for (x = 0; x < 8U; x++, voxel++)
            {
                int32_t p1, p2, p3, pc;
                int16_t w1, w2, w3, cloud;
                int32_t hueOffset;
                int32_t value8;
                uint16_t hue;
                uint16_t rq8, gq8, bq8;
                uint8_t r, g, b;

                p1 = (int32_t)x * 3200 + (int32_t)y * 1800 +
                     (int32_t)z * 2600 + phase1;
                w1 = sin16_q8((uint16_t)p1);

                p2 = (int32_t)x * 1400 - (int32_t)y * 2900 +
                     (int32_t)z * 2100 + phase2 + ((int32_t)w1 >> 2);
                w2 = sin16_q8((uint16_t)p2);

                p3 = -(int32_t)x * 2500 + (int32_t)y * 1300 +
                      (int32_t)z * 3300 + phase3 + ((int32_t)w2 >> 2);
                w3 = sin16_q8((uint16_t)p3);

                hueOffset = ((int32_t)w1 >> 1) +
                            ((int32_t)w2 >> 2) +
                            ((int32_t)w3 >> 2);
                hue = (uint16_t)((int32_t)huePhase + hueOffset);

                pc = (int32_t)x * 1900 + (int32_t)y * 2400 -
                     (int32_t)z * 1700 + phase4 +
                     ((int32_t)w1 >> 3) - ((int32_t)w3 >> 3);
                cloud = sin16_q8((uint16_t)pc);

                value8 = 185 + (((int32_t)cloud + ((int32_t)w2 >> 1)) >> 9);
                if (value8 < 105) value8 = 105;
                if (value8 > 255) value8 = 255;

                /* Full saturation removes the pastel wash from cloud v1. */
                hue16_to_rgb_q8(hue, (uint16_t)(value8 << 8),
                                &rq8, &gq8, &bq8);

                rq8 = apply_brightness_q8(rq8);
                gq8 = apply_brightness_q8(gq8);
                bq8 = apply_brightness_q8(bq8);

                r = dither_q8_to_u8(rq8, &ditherR[voxel]);
                g = dither_q8_to_u8(gq8, &ditherG[voxel]);
                b = dither_q8_to_u8(bq8, &ditherB[voxel]);
                set(x, y, z, r, g, b);
            }
        }
    }
}

/* ===== MAIN ===== */
int main(void)
{
    uint32_t i = 0;
    uint16_t flashChip;
    #ifdef doUSB
    RCC_ClocksTypeDef myClock;
    #endif
    debug = 0;

    SystemCoreClockUpdate();
    NVIC_Configuration();
    cube_random(8);
    setup();
    IR_ENC_Init(128);
    JTAGDisable();
    W25X_GPIO_Config();
    W25X_Init();

    /* --- detect board / flash (unchanged) --- */
    flashChip = SPI_Flash_ReadID();
    if (flashChip == 0XFFFF)
    {
        bjCubeType = 1;
        W25X_GPIO_BJConfig();
        W25X_Init();
        flashChip = SPI_Flash_ReadID();
        if (flashChip != 0XEF16)
        {
            if (debug==1) { hasUSB=1; USART_Configuration(115200); usb_printf("\r\nHi There!!\r\n"); }
            bjCubeType=0; ledCubeType=1; JTAGEnable();
        }
        else
        {
            ledCubeType=0;
            if (debug==1) { hasUSB=1; USART_Configuration(115200); usb_printf("\r\nHi There!!\r\n"); }
        }
    }
    else
    {
        ledCubeType=0;
    }

    /* --- core init (unchanged) --- */
    sysTickInit();
    exfuns_init();
    mem_init();
    populateDMAGrid(1);

    fileinfo.lfsize = _MAX_LFN * 2 + 1;
    fileinfo.lfname = mymalloc(fileinfo.lfsize);

    i = 0;
    while (SD_Initialize()) { if (++i > 60) break; }
    if (i < 60)
    {
        hasSD = 1;
        f_mount(fs[0], "0:", 1);
        mf_readConfigFile("0:\\config.txt");
        if (bjCubeType==2 && (cubeWIFI==1 || cubeWIFI==10))
        {
            cubeWIFI=10;
            if (debug==1) { cubeWIFI=0; DMAAddress=99; }
        }
    }

    initRGBOrder();

    if (ledCubeType==1) { GPIO_AuraConfiguration(); fft_AuraADC_Init(); }
    else
    {
        GPIO_Configuration();
        fft_ADC_Init();
        if (SPI_Flash_ReadID() == 0XEF16)
        {
            SPI_Flash_Read(W25QxxTempAA, 0, 3);
            W25QxxFrame = W25QxxTempAA[2];
            W25QxxSection = W25QxxTempAA[0] + (W25QxxTempAA[1] << 8);
        }
    }

    if (bjCubeType==1 && cubeWIFI) DMAAddress=99;
    if (cubeWIFI==10) DMAAddress=99;

    if (DMAAddress < 10) { if (DMAAddress!=0) welcome += 10; setupDMA(); }
    else
    {
        if (debug==0)
        {
            if (bjCubeType==1) USART2_Configuration();
            else               USART1_Configuration();
        }
    }

    resetI2C();
    if (initI2C()==0)
    {
        if (hasLCD) lcdInit();
        if (hasBME280) BME280Init();
        if (hasSHT31)  sht31Init();
        if (hasDS3231==10)
        {
            initDS3231();
            if (hasDS3231 && !hasBME280 && !hasSHT31) temperature = DS3231_ReadTemp();
        }
    }

    if (cubeWIFI)
    {
        if (bjCubeType==1) { ESP8266_USART=USART2; USART2_WIFIConfiguration(); USART2_WIFISendDMAConfiguration(); }
        else if (cubeWIFI==10) { ESP8266_USART=USART1; USART1_WIFIConfiguration(); USART1_WIFISendDMAConfiguration(); }
        else { USART3_Configuration(); USART3_WIFISendDMAConfiguration(); }
    }

    if (hasWIFI != 4) WIFICopyMode = 0;

    /* Start with both display buffers dark. */
    eNum = 0;
    showLED = 0;
    setLED = 0;
    setAll(0, 0, 0);
    setLED = 2;
    setAll(0, 0, 0);
    requestedShowLED = 0xFF;

    Timer2_Init(ScanFosc,ScanTime);

    /* Timer4 remains disabled.  IR decoding uses TIM1 independently. */
    {
        uint8_t backBuffer = 2U;
        uint32_t lastFrame = millis();

        /* Cloud phases are maintained in Q16.8 so slow-speed settings retain
         * fractional motion instead of stepping. */
        uint32_t phase1Q8 = 0x0000UL << 8;
        uint32_t phase2Q8 = 0x41A7UL << 8;
        uint32_t phase3Q8 = 0x93D5UL << 8;
        uint32_t phase4Q8 = 0xD26BUL << 8;
        uint32_t cloudHueQ8 = 0x1200UL << 8;

        /* Solid hue uses Q16.16.  At speed=100 this is one full turn in
         * approximately 60 seconds, matching the first smooth hue sketch. */
        uint32_t solidHueQ16 = 0U;
        const uint32_t solidHueBaseStepQ16 = 1145324UL;


        /* Breathing ball: phase begins at sine minimum, so the first frame is
         * the small centered ball.  Q16.8 keeps low speed settings smooth. */
        uint32_t ballBreathPhaseQ8 = 0xC000UL << 8;
        uint32_t ballHueQ8 = 0U;
        int16_t ballCenterXQ8 = 896; /* 3.5 voxels */
        int16_t ballCenterYQ8 = 896;
        int16_t ballCenterZQ8 = 896;
        int8_t ballDirX = 1;
        int8_t ballDirY = 1;
        int8_t ballDirZ = -1;

        power = 1;
        eNum = 0;

        while (1)
        {
            uint32_t now = millis();

            if ((uint32_t)(now - lastFrame) >= EFFECT_FRAME_MS)
            {
                uint8_t speedIdx;
                uint16_t speedPct;
                lastFrame = now;

                speedIdx = effectSpeedIndex;
                if (speedIdx >= SPEED_LEVEL_COUNT) speedIdx = (uint8_t)(SPEED_LEVEL_COUNT - 1U);
                speedPct = speedPercentTable[speedIdx];

                if (!effectPowerEnabled)
                {
                    setLED = backBuffer;
                    setAll(0, 0, 0);
                }
                else if (cubeEffectMode == 0U)
                {
                    render_solid_hue_frame(backBuffer, (uint16_t)(solidHueQ16 >> 16));
                }
                else if (cubeEffectMode == 1U)
                {
                    render_cloud_frame(backBuffer,
                                       (uint16_t)(phase1Q8 >> 8),
                                       (uint16_t)(phase2Q8 >> 8),
                                       (uint16_t)(phase3Q8 >> 8),
                                       (uint16_t)(phase4Q8 >> 8),
                                       (uint16_t)(cloudHueQ8 >> 8));
                }
                else if (cubeEffectMode == 2U)
                {
                    render_manual_rgb_frame(backBuffer);
                }
                else if (cubeEffectMode == 3U)
                {
                    render_diagnostic_frame(backBuffer);
                }
                else
                {
                    int16_t cx = ballBounceEnabled ? ballCenterXQ8 : 896;
                    int16_t cy = ballBounceEnabled ? ballCenterYQ8 : 896;
                    int16_t cz = ballBounceEnabled ? ballCenterZQ8 : 896;
                    render_ball_frame(backBuffer,
                                      (uint16_t)(ballBreathPhaseQ8 >> 8),
                                      (uint16_t)(ballHueQ8 >> 8),
                                      cx, cy, cz);
                }

                requestShowLEDSwap(backBuffer);
                while (requestedShowLED != 0xFF)
                {
                    /* TIM2 keeps displaying the previous complete frame. */
                }
                backBuffer = (backBuffer == 0U) ? 2U : 0U;

                if (effectPowerEnabled && !effectPaused)
                {
                    /* Fixed-point speed scaling keeps the low speed choices
                     * smooth instead of dropping phase increments to zero. */
                    phase1Q8 += (uint32_t)((19UL * 256UL * speedPct) / 100UL);
                    phase2Q8 += (uint32_t)((29UL * 256UL * speedPct) / 100UL);
                    phase3Q8 += (uint32_t)((43UL * 256UL * speedPct) / 100UL);
                    phase4Q8 += (uint32_t)((17UL * 256UL * speedPct) / 100UL);
                    cloudHueQ8 += (uint32_t)((11UL * 256UL * speedPct) / 100UL);
                    solidHueQ16 += (uint32_t)((solidHueBaseStepQ16 / 100UL) * speedPct);

                    /* About a 6-second breathing cycle at normal speed.
                     * Hue takes about two breaths for one complete rotation. */
                    ballBreathPhaseQ8 += (uint32_t)((175UL * 256UL * speedPct) / 100UL);
                    ballHueQ8 += (uint32_t)((90UL * 256UL * speedPct) / 100UL);

                    if (cubeEffectMode == 4U && ballBounceEnabled)
                    {
                        int16_t stepX = (int16_t)((9U * speedPct) / 100U);
                        int16_t stepY = (int16_t)((7U * speedPct) / 100U);
                        int16_t stepZ = (int16_t)((11U * speedPct) / 100U);
                        const int16_t minCenter = 192;   /* 0.75 voxel */
                        const int16_t maxCenter = 1600;  /* 6.25 voxels */
                        if (stepX < 1) stepX = 1;
                        if (stepY < 1) stepY = 1;
                        if (stepZ < 1) stepZ = 1;

                        ballCenterXQ8 = (int16_t)(ballCenterXQ8 + ballDirX * stepX);
                        ballCenterYQ8 = (int16_t)(ballCenterYQ8 + ballDirY * stepY);
                        ballCenterZQ8 = (int16_t)(ballCenterZQ8 + ballDirZ * stepZ);

                        if (ballCenterXQ8 <= minCenter) { ballCenterXQ8 = minCenter; ballDirX = 1; }
                        else if (ballCenterXQ8 >= maxCenter) { ballCenterXQ8 = maxCenter; ballDirX = -1; }
                        if (ballCenterYQ8 <= minCenter) { ballCenterYQ8 = minCenter; ballDirY = 1; }
                        else if (ballCenterYQ8 >= maxCenter) { ballCenterYQ8 = maxCenter; ballDirY = -1; }
                        if (ballCenterZQ8 <= minCenter) { ballCenterZQ8 = minCenter; ballDirZ = 1; }
                        else if (ballCenterZQ8 >= maxCenter) { ballCenterZQ8 = maxCenter; ballDirZ = -1; }
                    }
                }
            }
        }
    }

}
