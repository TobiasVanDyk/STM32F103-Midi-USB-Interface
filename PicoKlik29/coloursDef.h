// coloursDef.h
//////////////////////////////////////////////////////
// RGB565 Color Picker
// http://www.barth-dev.de/online/rgb565-color-picker/
//////////////////////////////////////////////////////
#define Black    0x0000      /*   0,   0,   0 */
#define Navy     0x000F      /*   0,   0, 128 */
#define DGreen   0x03E0      /*   0, 128,   0 */
#define DCyan    0x03EF      /*   0, 128, 128 */
#define Maroon   0x7800      /* 128,   0,   0 */
#define Purple   0x780F      /* 128,   0, 128 */
#define Olive    0x7BE0      /* 128, 128,   0 */
#define LGrey    0xD69A      /* 211, 211, 211 */
#define DGrey    0x7BEF      /* 128, 128, 128 */
#define Blue     0x001F      /*   0,   0, 255 */
#define Green    0x07E0      /*   0, 255,   0 */
#define Cyan     0x07FF      /*   0, 255, 255 */
#define Red      0xF800      /* 255,   0,   0 */
#define Magenta  0xF81F      /* 255,   0, 255 */
#define Yellow   0xFFE0      /* 255, 255,   0 */
#define White    0xFFFF      /* 255, 255, 255 */
#define Orange   0xFDA0      /* 255, 180,   0 */
#define Green1   0x4C63      // 180, 255,   0 
#define Green2   0x26C8      
#define Green3   0x1750
#define Green4   0x65C8
#define Green5   0x5527
#define Pink     0xFE19      /* 255, 192, 203 */ 
#define Brown    0x9A60      /* 150,  75,   0 */
#define Gold     0xFEA0      /* 255, 215,   0 */
#define Silver   0xC618      /* 192, 192, 192 */
#define Skyblue  0x867D      /* 135, 206, 235 */
#define Violet   0x915C      /* 180,  46, 226 */

// 25 button colours used in Layout 1 = IThru Routing and Layout 2 = Midi Jacks Routing
uint16_t Colours12[30] = {Black, Black, Black,  Black, Black, Blue,   // Routing L1 + L2
                          Black, Black, Black,  Black, Black, Blue,   // Slots
                          Black, Black, Black,  Black, Black, Blue,
                          Black, Black, Black,  Black, Black, Blue,   // Pipes
                          Black, Black, Black,  Black, Black, Blue }; // Option

// Two sets of 12 button colours used in Layouts 3 and 4
uint16_t Colours34[2][12] = {Violet, Violet, Violet, Violet,       // L3
                             Violet, Violet, Violet, Blue,   
                             Violet, Violet, Violet, Violet, 
                             Brown,  Brown,  Brown,  Brown,        // L4
                             Brown,  Brown,  Brown,  Maroon,     
                             Brown,  Brown,  Brown,  Brown, };
               
uint16_t keyColor12[30] = {};
uint16_t keyColor34[12] = {};

uint16_t BackgroundColor[4] = {Black, Black, Black, Black};   

uint16_t ButtonOutlineColor[4] = {White, White, White, White}; 

uint16_t LabelColor[4] = {White, White, White, White}; 
uint16_t XPointLabelColor = White; 
 
