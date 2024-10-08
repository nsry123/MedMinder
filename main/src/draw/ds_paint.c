#include "ds_paint.h"
#include "ds_screen.h"
#include "ds_spi.h"

PAINT Paint;

ROTATE_IMAGE Epd_Rotate = ROTATE_90;

uint8_t UTF8toUnicode(uint8_t *ch, uint16_t *_unicode)
{
    uint8_t *p = NULL ,n = 0;
    uint32_t e = 0;
    p = ch;
    if(1)//p == NULL
    {
            if(*p >= 0xfc)
            {
                    /*6:<11111100>*/
                    e  = (p[0] & 0x01) << 30;
                    e |= (p[1] & 0x3f) << 24;
                    e |= (p[2] & 0x3f) << 18;
                    e |= (p[3] & 0x3f) << 12;
                    e |= (p[4] & 0x3f) << 6;
                    e |= (p[5] & 0x3f);
                    n = 6;
            }
            else if(*p >= 0xf8)
            {
                    /*5:<11111000>*/
                    e = (p[0] & 0x03) << 24;
                    e |= (p[1] & 0x3f) << 18;
                    e |= (p[2] & 0x3f) << 12;
                    e |= (p[3] & 0x3f) << 6;
                    e |= (p[4] & 0x3f);
                    n = 5;
            }
            else if(*p >= 0xf0)
            {
                    /*4:<11110000>*/
                    e = (p[0] & 0x07) << 18;
                    e |= (p[1] & 0x3f) << 12;
                    e |= (p[2] & 0x3f) << 6;
                    e |= (p[3] & 0x3f);
                    n = 4;
            }
            else if(*p >= 0xe0)
            {
                    /*3:<11100000>*/
                    e = (p[0] & 0x0f) << 12;
                    e |= (p[1] & 0x3f) << 6;
                    e |= (p[2] & 0x3f);
                    n = 3;
            }
            else if(*p >= 0xc0)
            {
                    /*2:<11000000>*/
                    e = (p[0] & 0x1f) << 6;
                    e |= (p[1] & 0x3f);
                    n = 2;
            }
            else
            {
                    e = p[0];           
                    n = 1;
            }
            *_unicode = e;
    }
    return n;
}

/******************************************************************************
function: Create _Image
parameter:
    _image   :   Pointer to the _image cache
    width   :   The width of the picture
    Height  :   The height of the picture
    Color   :   Whether the picture is inverted
******************************************************************************/
void Paint_NewImage(UBYTE *_image, UWORD Width, UWORD Height, UWORD Rotate, UWORD Color)
{
    Paint._Image = NULL;
    Paint._Image = _image;

    Paint.WidthMemory = Width;
    Paint.HeightMemory = Height;
    Paint.Color = Color;    
    Paint.Scale = 2;
    
    Paint.WidthByte = (Width % 8 == 0)? (Width / 8 ): (Width / 8 + 1);
    Paint.HeightByte = Height;    
//    printf("WidthByte = %d, HeightByte = %d\r\n", Paint.WidthByte, Paint.HeightByte);
//    printf(" EPD_WIDTH / 8 = %d\r\n",  122 / 8);
   
    Paint.Rotate = Rotate;
    Paint.Mirror = MIRROR_NONE;
    
    if(Rotate == ROTATE_0 || Rotate == ROTATE_180) {
        Paint.Width = Width;
        Paint.Height = Height;
    } else {
        Paint.Width = Height;
        Paint.Height = Width;
    }
}

/******************************************************************************
function: Select _Image
parameter:
    _image : Pointer to the _image cache
******************************************************************************/
void Paint_SelectImage(UBYTE *_image)
{
    Paint._Image = _image;
}


/******************************************************************************
function: Draw Pixels
parameter:
    Xpoint : At point X
    Ypoint : At point Y
    Color  : Painted colors
******************************************************************************/
void Paint_SetPixel(UWORD Xpoint, UWORD Ypoint, UWORD Color)
{
    if(Xpoint > Paint.Width || Ypoint > Paint.Height){
        printf("Exceeding display boundaries Xpoint*Ypoint=%d*%d\r\n",Xpoint,Ypoint);
        return;
    }      
    UWORD X, Y;

    switch(Paint.Rotate) {
    case 0:
        X = Xpoint;
        Y = Ypoint;  
        break;
    case 90:
        X = Paint.WidthMemory - Ypoint - 1;
        Y = Xpoint;
        break;
    case 180:
        X = Paint.WidthMemory - Xpoint - 1;
        Y = Paint.HeightMemory - Ypoint - 1;
        break;
    case 270:
        X = Ypoint;
        Y = Paint.HeightMemory - Xpoint - 1;
        break;
    default:
        return;
    }
    
    switch(Paint.Mirror) {
    case MIRROR_NONE:
        break;
    case MIRROR_HORIZONTAL:
        X = Paint.WidthMemory - X - 1;
        break;
    case MIRROR_VERTICAL:
        Y = Paint.HeightMemory - Y - 1;
        break;
    case MIRROR_ORIGIN:
        X = Paint.WidthMemory - X - 1;
        Y = Paint.HeightMemory - Y - 1;
        break;
    default:
        return;
    }

    if(X > Paint.WidthMemory || Y > Paint.HeightMemory){
        printf("Exceeding display boundaries X*Y=%d*%d\r\n",X,Y);
        return;
    }
    
    if(Paint.Scale == 2){
        UDOUBLE Addr = X / 8 + Y * Paint.WidthByte;
        UBYTE Rdata = Paint._Image[Addr];
        if(Color == BLACK)
            Paint._Image[Addr] = Rdata & ~(0x80 >> (X % 8));
        else
            Paint._Image[Addr] = Rdata | (0x80 >> (X % 8));
    }else if(Paint.Scale == 4){
        UDOUBLE Addr = X / 4 + Y * Paint.WidthByte;
        Color = Color % 4;//Guaranteed color scale is 4  --- 0~3
        UBYTE Rdata = Paint._Image[Addr];
        
        Rdata = Rdata & (~(0xC0 >> ((X % 4)*2)));
        Paint._Image[Addr] = Rdata | ((Color << 6) >> ((X % 4)*2));
    }
}

/******************************************************************************
function: Clear the color of the picture
parameter:
    Color : Painted colors
******************************************************************************/
void Paint_Clear(UWORD Color)
{
    for (UWORD Y = 0; Y < Paint.HeightByte; Y++) {
        for (UWORD X = 0; X < Paint.WidthByte; X++ ) {//8 pixel =  1 byte
            UDOUBLE Addr = X + Y*Paint.WidthByte;
            Paint._Image[Addr] = Color;
        }
    }
}

/******************************************************************************
function: Clear the color of a window
parameter:
    Xstart : x starting point
    Ystart : Y starting point
    Xend   : x end point
    Yend   : y end point
    Color  : Painted colors
******************************************************************************/
void Paint_ClearWindows(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend, UWORD Color)
{
    UWORD X, Y;
    for (Y = Ystart; Y < Yend; Y++) {
        for (X = Xstart; X < Xend; X++) {//8 pixel =  1 byte
            Paint_SetPixel(X, Y, Color);
        }
    }
}

/******************************************************************************
function: Draw Point(Xpoint, Ypoint) Fill the color
parameter:
    Xpoint    : The Xpoint coordinate of the point
    Ypoint    : The Ypoint coordinate of the point
    Color   : Painted color
    Dot_Pixel : point size
    Dot_Style : point Style
******************************************************************************/
void Paint_DrawPoint(UWORD Xpoint, UWORD Ypoint, UWORD Color,
                     DOT_PIXEL Dot_Pixel, DOT_STYLE Dot_Style)
{
    if (Xpoint > Paint.Width || Ypoint > Paint.Height) {
        printf("Paint_DrawPoint Input exceeds the normal display range\r\n");
        printf("Xpoint = %d , Paint.Width = %d  \r\n ",Xpoint,Paint.Width);
        printf("Ypoint = %d , Paint.Height = %d  \r\n ",Ypoint,Paint.Height);
        return;
    }

    int16_t XDir_Num , YDir_Num;
    if (Dot_Style == DOT_FILL_AROUND) {
        for (XDir_Num = 0; XDir_Num < 2 * Dot_Pixel - 1; XDir_Num++) {
            for (YDir_Num = 0; YDir_Num < 2 * Dot_Pixel - 1; YDir_Num++) {
                // if((Xpoint + XDir_Num - Dot_Pixel) < 0 || (Ypoint + YDir_Num - Dot_Pixel) < 0)
                //     break;
                // printf("x = %d, y = %d\r\n", Xpoint + XDir_Num - Dot_Pixel, Ypoint + YDir_Num - Dot_Pixel);
                Paint_SetPixel(Xpoint + XDir_Num - Dot_Pixel, Ypoint + YDir_Num - Dot_Pixel, Color);
            }
        }
    } else {
        for (XDir_Num = 0; XDir_Num <  Dot_Pixel; XDir_Num++) {
            for (YDir_Num = 0; YDir_Num <  Dot_Pixel; YDir_Num++) {
                Paint_SetPixel(Xpoint + XDir_Num - 1, Ypoint + YDir_Num - 1, Color);
            }
        }
    }
}

/******************************************************************************
function: Draw a line of arbitrary slope
parameter:
    Xstart ：Starting Xpoint point coordinates
    Ystart ：Starting Xpoint point coordinates
    Xend   ：End point Xpoint coordinate
    Yend   ：End point Ypoint coordinate
    Color  ：The color of the line segment
    Line_width : Line width
    Line_Style: Solid and dotted lines
******************************************************************************/
void Paint_DrawLine(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend,
                    UWORD Color, DOT_PIXEL Line_width, LINE_STYLE Line_Style)
{
    if (Xstart > Paint.Width || Ystart > Paint.Height ||
        Xend > Paint.Width || Yend > Paint.Height) {
        printf("Paint_DrawLine Input exceeds the normal display range\r\n");
        return;
    }

    UWORD Xpoint = Xstart;
    UWORD Ypoint = Ystart;
    int dx = (int)Xend - (int)Xstart >= 0 ? Xend - Xstart : Xstart - Xend;
    int dy = (int)Yend - (int)Ystart <= 0 ? Yend - Ystart : Ystart - Yend;

    // Increment direction, 1 is positive, -1 is counter;
    int XAddway = Xstart < Xend ? 1 : -1;
    int YAddway = Ystart < Yend ? 1 : -1;

    //Cumulative error
    int Esp = dx + dy;
    char Dotted_Len = 0;

    for (;;) {
        Dotted_Len++;
        //Painted dotted line, 2 point is really virtual
        if (Line_Style == LINE_STYLE_DOTTED && Dotted_Len % 3 == 0) {
            //Debug("LINE_DOTTED\r\n");
            Paint_DrawPoint(Xpoint, Ypoint, IMAGE_BACKGROUND, Line_width, DOT_STYLE_DFT);
            Dotted_Len = 0;
        } else {
            Paint_DrawPoint(Xpoint, Ypoint, Color, Line_width, DOT_STYLE_DFT);
        }
        if (2 * Esp >= dy) {
            if (Xpoint == Xend)
                break;
            Esp += dy;
            Xpoint += XAddway;
        }
        if (2 * Esp <= dx) {
            if (Ypoint == Yend)
                break;
            Esp += dx;
            Ypoint += YAddway;
        }
    }
}

/******************************************************************************
function: Draw a rectangle
parameter:
    Xstart ：Rectangular  Starting Xpoint point coordinates
    Ystart ：Rectangular  Starting Xpoint point coordinates
    Xend   ：Rectangular  End point Xpoint coordinate
    Yend   ：Rectangular  End point Ypoint coordinate
    Color  ：The color of the Rectangular segment
    Line_width: Line width
    Draw_Fill : Whether to fill the inside of the rectangle
******************************************************************************/
void Paint_DrawRectangle(UWORD Xstart, UWORD Ystart, UWORD Xend, UWORD Yend,
                         UWORD Color, DOT_PIXEL Line_width, DRAW_FILL Draw_Fill)
{
    if (Xstart > Paint.Width || Ystart > Paint.Height ||
        Xend > Paint.Width || Yend > Paint.Height) {
        printf("Input exceeds the normal display range\r\n");
        return;
    }

    if (Draw_Fill) {
        UWORD Ypoint;
        for(Ypoint = Ystart; Ypoint < Yend; Ypoint++) {
            Paint_DrawLine(Xstart, Ypoint, Xend, Ypoint, Color , Line_width, LINE_STYLE_SOLID);
        }
    } else {
        Paint_DrawLine(Xstart, Ystart, Xend, Ystart, Color, Line_width, LINE_STYLE_SOLID);
        Paint_DrawLine(Xstart, Ystart, Xstart, Yend, Color, Line_width, LINE_STYLE_SOLID);
        Paint_DrawLine(Xend, Yend, Xend, Ystart, Color, Line_width, LINE_STYLE_SOLID);
        Paint_DrawLine(Xend, Yend, Xstart, Yend, Color, Line_width, LINE_STYLE_SOLID);
    }
}

/******************************************************************************
function: Use the 8-point method to draw a circle of the
            specified size at the specified position->
parameter:
    X_Center  ：Center X coordinate
    Y_Center  ：Center Y coordinate
    Radius    ：circle Radius
    Color     ：The color of the ：circle segment
    Line_width: Line width
    Draw_Fill : Whether to fill the inside of the Circle
******************************************************************************/
void Paint_DrawCircle(UWORD X_Center, UWORD Y_Center, UWORD Radius,
                      UWORD Color, DOT_PIXEL Line_width, DRAW_FILL Draw_Fill)
{
    if (X_Center > Paint.Width || Y_Center >= Paint.Height) {
        printf("Paint_DrawCircle Input exceeds the normal display range\r\n");
        return;
    }

    //Draw a circle from(0, R) as a starting point
    int16_t XCurrent, YCurrent;
    XCurrent = 0;
    YCurrent = Radius;

    //Cumulative error,judge the next point of the logo
    int16_t Esp = 3 - (Radius << 1 );

    int16_t sCountY;
    if (Draw_Fill == DRAW_FILL_FULL) {
        while (XCurrent <= YCurrent ) { //Realistic circles
            for (sCountY = XCurrent; sCountY <= YCurrent; sCountY ++ ) {
                Paint_DrawPoint(X_Center + XCurrent, Y_Center + sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//1
                Paint_DrawPoint(X_Center - XCurrent, Y_Center + sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//2
                Paint_DrawPoint(X_Center - sCountY, Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//3
                Paint_DrawPoint(X_Center - sCountY, Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//4
                Paint_DrawPoint(X_Center - XCurrent, Y_Center - sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//5
                Paint_DrawPoint(X_Center + XCurrent, Y_Center - sCountY, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//6
                Paint_DrawPoint(X_Center + sCountY, Y_Center - XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);//7
                Paint_DrawPoint(X_Center + sCountY, Y_Center + XCurrent, Color, DOT_PIXEL_DFT, DOT_STYLE_DFT);
            }
            if (Esp < 0 )
                Esp += 4 * XCurrent + 6;
            else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    } else { //Draw a hollow circle
        while (XCurrent <= YCurrent ) {
            Paint_DrawPoint(X_Center + XCurrent, Y_Center + YCurrent, Color, Line_width, DOT_STYLE_DFT);//1
            Paint_DrawPoint(X_Center - XCurrent, Y_Center + YCurrent, Color, Line_width, DOT_STYLE_DFT);//2
            Paint_DrawPoint(X_Center - YCurrent, Y_Center + XCurrent, Color, Line_width, DOT_STYLE_DFT);//3
            Paint_DrawPoint(X_Center - YCurrent, Y_Center - XCurrent, Color, Line_width, DOT_STYLE_DFT);//4
            Paint_DrawPoint(X_Center - XCurrent, Y_Center - YCurrent, Color, Line_width, DOT_STYLE_DFT);//5
            Paint_DrawPoint(X_Center + XCurrent, Y_Center - YCurrent, Color, Line_width, DOT_STYLE_DFT);//6
            Paint_DrawPoint(X_Center + YCurrent, Y_Center - XCurrent, Color, Line_width, DOT_STYLE_DFT);//7
            Paint_DrawPoint(X_Center + YCurrent, Y_Center + XCurrent, Color, Line_width, DOT_STYLE_DFT);//0

            if (Esp < 0 )
                Esp += 4 * XCurrent + 6;
            else {
                Esp += 10 + 4 * (XCurrent - YCurrent );
                YCurrent --;
            }
            XCurrent ++;
        }
    }
}

/******************************************************************************
function: Display the string
parameter:
    Xstart  ：X coordinate
    Ystart  ：Y coordinate
    pString ：The first address of the Chinese string and English
              string to be displayed
    Font    ：A structure pointer that displays a character size
    Color_Foreground : Select the foreground color
    Color_Background : Select the background color
******************************************************************************/
void Paint_DrawString_CN(UWORD Xstart, UWORD Ystart, const char * pString,
                        UWORD Color_Foreground, UWORD Color_Background)
{
    const char* pstr = pString;
    int i;
    uint8_t  bitmap[300];
    i = 0;
    uint16_t char_cn_un = 0;
    
    //当前中文字数
    uint16_t cn_data_num = 0;
    uint16_t cn_box_w = 0;
    //当前英文字数
    uint16_t en_data_num = 0;
    uint16_t en_box_w = 0;
    while (*(pstr+i) != '\0')
    {
        memset(&bitmap,0,sizeof(bitmap));
        UTF8toUnicode((uint8_t*)pstr+i,&char_cn_un);
        uint8_t box_w = 0,box_h = 0;
        uint8_t offset_x = 0,offset_y = 0;
        int size = ds_get_bitmap(char_cn_un,bitmap,&box_w,&box_h,&offset_x,&offset_y);
            
        uint8_t page_len = box_h;  //box_h
        uint8_t colnum_len = box_w/2; //box_w/2
        printf("page_len %d  colnum_len %d\n",page_len,colnum_len); 
        for(uint8_t Page = 0; Page < page_len; Page ++) //0-127
        { 
            for (uint8_t Column = 0; Column < colnum_len; Column++) //0-1
            {
                uint8_t tmp = bitmap[Page*colnum_len + Column]; 
                // printf("%x ",tmp);  
                for(int index = 0;index < 8 ;index += 4){
                    if (((tmp >> index) & 0x0f) >= 1){
                        Paint_SetPixel( Xstart + 2*Column + (1- index/4) + (en_box_w + cn_box_w)+(en_data_num+cn_data_num)*3 + offset_x,Ystart+ Page -offset_y -page_len    , Color_Background);
                    }else{
                        Paint_SetPixel(  Xstart + 2*Column + (1- index/4) + (en_box_w + cn_box_w)+(en_data_num+cn_data_num)*3 + offset_x,Ystart+ Page - offset_y -page_len  , Color_Foreground);
                    }
                }
            }
            // printf("\n"); 
        }

        if (*(pstr+i) >= 128){
            //中文4byte
            i+=3;
            cn_data_num ++;
            cn_box_w += box_w;
        }else{
            i++;
            en_data_num ++;
            en_box_w += box_w;
        }
    }
}

int UTF8toUnicode__(uint8_t* pInput, uint16_t* Unic)
{
    if (pInput == NULL || Unic == NULL) {
        return 0;
    }

    int len = 0;
    uint16_t unicode = 0;
    if (pInput[0] < 0x80) {
        // Single-byte ASCII
        *Unic = pInput[0];
        return 1;
    } else if ((pInput[0] & 0xE0) == 0xC0) {
        // Two-byte sequence
        if ((pInput[1] & 0xC0) != 0x80) return 0; // Invalid continuation byte
        unicode = ((pInput[0] & 0x1F) << 6) | (pInput[1] & 0x3F);
        *Unic = unicode;
        return 2;
    } else if ((pInput[0] & 0xF0) == 0xE0) {
        // Three-byte sequence
        if ((pInput[1] & 0xC0) != 0x80 || (pInput[2] & 0xC0) != 0x80) return 0; // Invalid continuation bytes
        unicode = ((pInput[0] & 0x0F) << 12) | ((pInput[1] & 0x3F) << 6) | (pInput[2] & 0x3F);
        *Unic = unicode;
        return 3;
    } else {
        // Invalid UTF-8 encoding
        return 0;
    }
}
// void Paint_DrawString_CN_scaled(UWORD Xstart, UWORD Ystart, const char *pString,
//                                 UWORD Color_Foreground, UWORD Color_Background, float scale)
// {
//     if (pString == NULL) {
//         printf("Error: pString is NULL.\n");
//         return;
//     }

//     const char *pstr = pString;
//     int i = 0;
//     uint16_t char_cn_un = 0;

//     // Current number of Chinese and English characters
//     uint16_t cn_data_num = 0;
//     uint16_t cn_box_w = 0;
//     uint16_t en_data_num = 0;
//     uint16_t en_box_w = 0;

//     while (*(pstr + i) != '\0')
//     {
//         // Get Unicode code point and number of bytes consumed
//         int utf8_bytes = UTF8toUnicode((uint8_t *)pstr + i, &char_cn_un);
//         if (utf8_bytes <= 0) {
//             printf("Error: Invalid UTF-8 encoding at position %d.\n", i);
//             break; // Stop processing to prevent infinite loop
//         }

//         // First, get the size needed for the bitmap
//         uint8_t box_w = 0, box_h = 0;
//         uint8_t offset_x = 0, offset_y = 0;
//         int size = ds_get_bitmap(char_cn_un, NULL, &box_w, &box_h, &offset_x, &offset_y);
//         if (size <= 0) {
//             printf("Error: Failed to get bitmap size for character U+%04X.\n", char_cn_un);
//             i += utf8_bytes;
//             continue; // Skip this character
//         }

//         // Allocate bitmap buffer dynamically
//         uint8_t *bitmap = (uint8_t *)malloc(size);
//         if (bitmap == NULL) {
//             printf("Error: Insufficient memory to allocate bitmap for character U+%04X.\n", char_cn_un);
//             i += utf8_bytes;
//             continue; // Skip this character
//         }

//         // Now get the actual bitmap data
//         size = ds_get_bitmap(char_cn_un, bitmap, &box_w, &box_h, &offset_x, &offset_y);
//         if (size <= 0) {
//             printf("Error: Failed to get bitmap data for character U+%04X.\n", char_cn_un);
//             free(bitmap);
//             i += utf8_bytes;
//             continue; // Skip this character
//         }

//         // Rest of your drawing code remains the same...

//         // Remember to free the bitmap buffer after use
//         free(bitmap);

//         // Update cumulative widths based on the scaled character width
//         if ((uint8_t)*(pstr + i) >= 0x80)
//         {
//             // Multibyte character
//             i += utf8_bytes;
//             cn_data_num++;
//             cn_box_w += (int)(box_w * scale);
//         }
//         else
//         {
//             // Single-byte character (ASCII)
//             i++;
//             en_data_num++;
//             en_box_w += (int)(box_w * scale);
//         }
//     }
// }

void Paint_DrawString_CN_scaled(UWORD Xstart, UWORD Ystart, const char *pString,
                                UWORD Color_Foreground, UWORD Color_Background, float scale)
{
    if (pString == NULL) {
        printf("Error: pString is NULL.\n");
        return;
    }

    const char *pstr = pString;
    int i = 0;
    uint16_t char_cn_un = 0;

    // Current number of Chinese and English characters
    uint16_t cn_data_num = 0;
    uint16_t cn_box_w = 0;
    uint16_t en_data_num = 0;
    uint16_t en_box_w = 0;

    while (*(pstr + i) != '\0')
    {
        // Get Unicode code point and number of bytes consumed
        int utf8_bytes = UTF8toUnicode((uint8_t *)pstr + i, &char_cn_un);
        if (utf8_bytes <= 0) {
            printf("Error: Invalid UTF-8 encoding at position %d.\n", i);
            break; // Stop processing to prevent infinite loop
        }

        // First, get the size needed for the bitmap
        uint8_t box_w = 0, box_h = 0;
        uint8_t offset_x = 0, offset_y = 0;
        int size = ds_get_bitmap(char_cn_un, NULL, &box_w, &box_h, &offset_x, &offset_y);
        if (size <= 0) {
            printf("Error: Failed to get bitmap size for character U+%04X.\n", char_cn_un);
            i += utf8_bytes;
            continue; // Skip this character
        }

        // Allocate bitmap buffer dynamically
        uint8_t *bitmap = (uint8_t *)malloc(size);
        if (bitmap == NULL) {
            printf("Error: Insufficient memory to allocate bitmap for character U+%04X.\n", char_cn_un);
            i += utf8_bytes;
            continue; // Skip this character
        }

        // Now get the actual bitmap data
        size = ds_get_bitmap(char_cn_un, bitmap, &box_w, &box_h, &offset_x, &offset_y);
        if (size <= 0) {
            printf("Error: Failed to get bitmap data for character U+%04X.\n", char_cn_un);
            free(bitmap);
            i += utf8_bytes;
            continue; // Skip this character
        }


        // Ensure box dimensions are valid
        if (box_w == 0 || box_h == 0) {
            printf("Error: Invalid dimensions for character U+%04X.\n", char_cn_un);
            free(bitmap);
            i += utf8_bytes;
            continue; // Skip this character
        }

        uint8_t page_len = box_h;       // Height of the character
        uint8_t colnum_len = box_w / 2; // Width of the character in bytes

        // Compute scaled dimensions
        int scaled_box_w = (int)(box_w * scale);
        int scaled_box_h = (int)(box_h * scale);

        // Adjust cumulative widths and spacing
        int spacing = (int)(3 * scale);

        int total_en_box_w = en_box_w;
        int total_cn_box_w = cn_box_w;

        int x_offset = Xstart + total_en_box_w + total_cn_box_w + (en_data_num + cn_data_num) * spacing + (int)(offset_x * scale);
        int y_offset = Ystart - (int)(offset_y * scale) - (int)(page_len * scale);

        // Loop over scaled bitmap
        for (int y_s = 0; y_s < scaled_box_h; y_s++)
        {
            // Map back to original y position
            float y_o = y_s / scale;
            int y_o_int = (int)y_o;
            if (y_o_int < 0 || y_o_int >= page_len)
                continue;

            for (int x_s = 0; x_s < scaled_box_w; x_s++)
            {
                // Map back to original x position
                float x_o = x_s / scale;
                int x_o_int = (int)x_o;
                if (x_o_int < 0 || x_o_int >= box_w)
                    continue;

                int Column = x_o_int / 2;
                int bit_index = x_o_int % 2;

                int index = (1 - bit_index) * 4;

                // Check array bounds
                int bitmap_index = y_o_int * colnum_len + Column;
                if (bitmap_index < 0 || bitmap_index >= size)
                {
                    continue;
                }

                uint8_t tmp = bitmap[bitmap_index];

                int pixel_on = ((tmp >> index) & 0x0F) >= 1;

                int x_pos = x_offset + x_s;
                int y_pos = y_offset + y_s;

                // Ensure x_pos and y_pos are within screen bounds
                if (x_pos < 0 || x_pos >= 200 || y_pos < 0 || y_pos >= 200)
                {
                    continue;
                }
                if (pixel_on)
                    Paint_SetPixel(x_pos, y_pos, Color_Background);
                else
                    Paint_SetPixel(x_pos, y_pos, Color_Foreground);
            }
        }

        // Free the bitmap buffer
        free(bitmap);

        // Update cumulative widths based on the scaled character width
        if ((uint8_t)*(pstr + i) >= 0x80)
        {
            // Multibyte character
            i += utf8_bytes;
            cn_data_num++;
            cn_box_w += scaled_box_w;
        }
        else
        {
            // Single-byte character (ASCII)
            i++;
            en_data_num++;
            en_box_w += scaled_box_w;
        }
    }
}


void Paint_DrawString_piture(UWORD Xstart, UWORD Ystart, const unsigned char *pbuf ,int plen,int pxsize,
                        UWORD Color_Foreground, UWORD Color_Background)
{
    uint8_t width = (pxsize%8 == 0) ? (pxsize/8) : (pxsize/8+1);
    uint8_t height = plen / width;
    for(int y = 0;y < height; y ++){
        for(int x = 0;x < width ;x ++){
            for(int index = 0;index < 8;index ++){
                if(((pbuf[y*width + x]>>(7-index)) & 0x01) == 1){
                    Paint_SetPixel( Xstart + x*8 + index ,Ystart+ y , Color_Background);
                }else{
                    Paint_SetPixel(  Xstart + x*8 + index,Ystart+ y , Color_Foreground);
                }
            }
        }
    }
}

//图片全刷
void ds_paint_image(void){
	unsigned int i;
	spi_send_cmd(0x24);
	for(i=0;i<5000;i++){
		spi_send_data(Paint._Image[i]);  
	}  	 		
}

unsigned char gImage_last_page[5000];

void ds_paint_last_page(void){
	unsigned int i;
	for(i=0;i<5000;i++){
		spi_send_data(gImage_last_page[i]);  
	}  	 		
}

void ds_paint_image_new(){
    unsigned int i;
    spi_send_cmd(0x24); 
    for(i=0;i<5000;i++){
		spi_send_data(Paint._Image[i]);  
	}  	 	
}

void ds_paint_image_copy(){
    memcpy(gImage_last_page,Paint._Image,5000);
}
