#ifndef __COLORMATRIX2_H__
#define __COLORMATRIX2_H__

/* Color matrix application rule

 1  | rr  gr  br  wr  | |  R  |   |  R' |
--- | rg  gg  bg  wg  | |  G  | = |  G' |
256 | rb  gb  bb  wb  | |  B  |   |  B' |
    | 0   0   0   256 | | 256 |   | 256 |

sum of each row must be 256
*/

struct ColorMatrix2
{
	short int matrix[3][4];
	ColorMatrix2& operator =(const ColorMatrix2& matrix);
	ColorMatrix2 operator *(const ColorMatrix2& matrix);
	ColorMatrix2& operator *=(const ColorMatrix2& matrix);
	ColorMatrix2& blend(ColorMatrix2& matrix,int opacity=128);
	inline ColorMatrix2& initialize(short int a00,short int a01,short int a02,short int a03,
									short int a10,short int a11,short int a12,short int a13,
									short int a20,short int a21,short int a22,short int a23)
	{
#define FE(i,j) matrix[i][j]=a##i##j;
		FE(0,0)FE(0,1)FE(0,2)FE(0,3)
		FE(1,0)FE(1,1)FE(1,2)FE(1,3)
		FE(2,0)FE(2,1)FE(2,2)FE(2,3)
#undef FE
		return *this;
	}
};

// Make picture black&white
const ColorMatrix2 CM_gray={{{86,85,85,0},{85,86,85,0},{85,85,86,0}}};

// Extract color channels from picture
const ColorMatrix2 CM_extract_red={{{256,0,0,0},{256,0,0,0},{256,0,0,0}}};
const ColorMatrix2 CM_extract_green={{{0,256,0,0},{0,256,0,0},{0,256,0,0}}};
const ColorMatrix2 CM_extract_blue={{{0,0,256,0},{0,0,256,0},{0,0,256,0}}};

// 0 indicates no gray, 256 indicates near full gray
ColorMatrix2& CM_partial_gray(int degree,ColorMatrix2& matrix=ColorMatrix2());

// 0 indicates no hue change
// 256 indicates hue change of 120 degrees
// 512 indicates hue change of 240 degrees
// 768 indicates hue change of 360 degrees(same as 0)
// NOT ACCURATE!!
ColorMatrix2& CM_hue_change(int degree,ColorMatrix2& matrix=ColorMatrix2());

// 0 indicates no change, 256 indicates full blackout
ColorMatrix2& CM_fadeout(int degree,ColorMatrix2& matrix=ColorMatrix2());

// 0 indicates no change, 256 indicates full whiteout
ColorMatrix2& CM_whiteout(int degree,ColorMatrix2& matrix=ColorMatrix2());

#endif //__COLORMATRIX2_H__