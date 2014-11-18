/*
**	$VER: SetPalette.h  Release V1.0 (18.09.93)
**
**	SetPalette file for changing color palette of screens
**
**  Programmed by : Raul A. Sobon.
**
**	(C) Copyright 1993 TreeSoft TM, Inc.
**	    All Rights Reserved
*/


#include <exec/types.h>
#include <exec/memory.h>
#include <exec/execbase.h>

#include <dos/dos.h>
#include <dos/dosextens.h>
#include <dos/stdio.h>				// doses stdio

#include <intuition/classes.h>
#include <intuition/gadgetclass.h>
#include <intuition/intuitionbase.h>

#include "SetPalette_globals.h"				// my private data

#include <proto/exec.h>				// use amiga library stuff
#include <proto/dos.h>
#include <proto/graphics.h>
#include <proto/intuition.h>
#include <proto/gadtools.h> 
#include <proto/diskfont.h> 
#include <proto/asl.h>

#include <stdio.h>					// and the thing we all use!
#include <string.h>

#define TOOLS_IDCMP          ARROWIDCMP | BUTTONIDCMP | CHECKBOXIDCMP |\
                             INTEGERIDCMP | LISTVIEWIDCMP | MXIDCMP |\
                             CYCLEIDCMP | PALETTEIDCMP | SCROLLERIDCMP |\
                             SLIDERIDCMP | STRINGIDCMP

/*
 * --- External refrenced data.
 */
ULONG             IClass;
UWORD			  Qualifier, Code, Prev_Code;
struct Gadget    *IObject;
APTR              MainVisualInfo;
struct TextAttr thinpaz8 = {
	( STRPTR )"thinpaz.font", 8, 0x00, 0x00 };
struct TextAttr   Topaz80;
struct Screen    *MainScreen;
struct TextFont	 *thinpazfont8;

#include	"SetPalette_rev.h"
static	UBYTE	verstr[]={ VERSTAG };


#define	FORM_ID	0x464f524d
#define	ILBM_ID	0x494c424d
#define	CMAP_ID	0x434d4150

static ULONG	iffchunk[]={
	FORM_ID,		// FORM
		0x4c,		// length of whole thing
	ILBM_ID,		// ILBM
		0x14,	0,0,0,0x2c2c,0,
	CMAP_ID,		// CMAP
		0x00,		// number of colors * 3
		0x00		// colors start here		(r,g,b...r,g,b)
	};		


/*
 * --- All my requesters use these tags
 */
struct TagItem               ReqTags[] = {
    WA_Left,                50l,
    WA_Top,                 50l,
    WA_Width,               310l,
    WA_Height,              128l,
    WA_IDCMP,               IDCMP_CLOSEWINDOW | TOOLS_IDCMP | IDCMP_VANILLAKEY | IDCMP_REFRESHWINDOW,
    WA_Flags,               WFLG_DRAGBAR | WFLG_DEPTHGADGET| WFLG_CLOSEGADGET | WFLG_ACTIVATE | WFLG_RMBTRAP | WFLG_SMART_REFRESH | WFLG_GIMMEZEROZERO,
    WA_Gadgets,             0l,
    WA_Title,               0l,
    WA_AutoAdjust,          TRUE,
    WA_CustomScreen,        0l,
    TAG_DONE };

/*
 * --- Palette window gadget ID's
 */
#define GD_RED           0
#define GD_GREEN         1
#define GD_BLUE          2
#define GD_PALETTE       3
#define GD_LOAD          4
#define GD_SAVE          5
#define GD_OK            6
#define GD_RESET         7
#define GD_CANCEL        8
#define GD_JUMP          9
#define GD_COPY         10
#define GD_EXCHANGE     11
#define GD_SPREAD       12

/*
 * --- Edit Mode ID's ---
 */
#define EM_COPY         1
#define EM_EXCHANGE     2
#define EM_SPREAD       3

/*
 * --- Program gadget pointers that needs to be changed.
 */
struct Window           *sp_Wnd   = NULL;
struct Gadget           *sp_Red, *sp_Green, *sp_Blue, *sp_Palette;
struct Gadget           *sp_GList = NULL;
struct Gadget			*g;
struct NewGadget		ng;
UWORD                   sp_Orig[32];			// 4 gun palette 32 entries (pre V39)
ULONG					sp_Orig32[256][3];		// 8 gun palette 256 entries

UWORD                   sp_Rc, sp_Gc, sp_Bc;

UBYTE                   *sp_Title = "Screen Palette © 1993 (Raul A. Sobon)";

/*
 * --- TagItems for the slider gadgets.
 */
struct TagItem  sp_PTags[] = {
    GTSL_LevelFormat,       (ULONG)"%03ld",
    GTSL_MaxLevelLen,       3L,
	GTSL_Min,				0L,
	GTSL_Max,				255L,
    TAG_DONE };


ULONG	    version=30;
ULONG	    GunsTemp1[3];
ULONG       GunsTemp2[3];
ULONG       Col12;
ULONG       Col12_2;
UWORD       r4, g4, b4;
UWORD       r4_2, g4_2, b4_2;
UBYTE       Edit_Mode;


// -------------------------------------------------------------------------------------------

long GUN8TO32( long col ){

	col = (col) | (col<<8);
	col = (col) | (col<<16);
	return (col);
}




/*
 * --- Set the slider levels accoording to color register 'reg'.
 */
void SetProp( long reg )
{
	ULONG	guns[3];
    UWORD   r, g, b, col;

	if ( version > 38) {
	    GetRGB32( MainScreen->ViewPort.ColorMap, (long)reg, 1, &guns[0] );
		r = sp_Rc = (guns[0]>>24)&0xff;
		g = sp_Gc = (guns[1]>>24)&0xff;
		b = sp_Bc = (guns[2]>>24)&0xff;
	}
	else {
	    col = GetRGB4( MainScreen->ViewPort.ColorMap, reg );
	    r = sp_Rc = (( col >> 8 ) & 0x0f )<<4;
	    g = sp_Gc = (( col >> 4 ) & 0x0f )<<4;
	    b = sp_Bc = ( col & 0x0f )<<4;
	}

    GT_SetGadgetAttrs( sp_Red,   sp_Wnd, NULL, GTSL_Level, r, TAG_DONE );
    GT_SetGadgetAttrs( sp_Green, sp_Wnd, NULL, GTSL_Level, g, TAG_DONE );
    GT_SetGadgetAttrs( sp_Blue,  sp_Wnd, NULL, GTSL_Level, b, TAG_DONE );
}




/*
 *  --- set the palette to original screen palette before mods
 */
void loadOrig( void ) {
	long	r;

	if ( version > 38 )
	    for ( r = 0; r < ( 1L << MainScreen->BitMap.Depth ); r++ )
	        SetRGB32( &MainScreen->ViewPort, r ,
				GUN8TO32( sp_Orig32[r][0] ),
				GUN8TO32( sp_Orig32[r][1] ),
				GUN8TO32( sp_Orig32[r][2] ) );
	else 
	    for ( r = 0; r < ( 1L << MainScreen->BitMap.Depth ); r++ )
	        SetRGB4( &MainScreen->ViewPort, (long)r,
				(sp_Orig[r]&0xf00)>>8,
				(sp_Orig[r]&0x0f0)>>4,
				(sp_Orig[r]&0x00f) );

}



/*
 * --- Reads a message from the window message port.
 * --- Returns TRUE if a message was read and puts the
 * --- message data in the globals. Return FALSE if there
 * --- was no message at the port.
 */
long ReadIMsg( struct Window *iwnd )
{
    struct IntuiMessage *imsg;

    if ( imsg = GT_GetIMsg( iwnd->UserPort )) {

        IClass      =   imsg->Class;
        Qualifier   =   imsg->Qualifier;
        Code        =   imsg->Code;
        IObject     =   imsg->IAddress;

        GT_ReplyIMsg( imsg );

        return TRUE;
    }
    return FALSE;
}



/*
 * --- Clears all message from a message port.
 */
void ClearMsgPort( struct MsgPort *mport )
{
    struct IntuiMessage  *msg;

    while ( msg = GT_GetIMsg( mport )) GT_ReplyIMsg( msg );
}





/*
 * --- Open lots of libraries that I need.
 */
long OpenLibraries( void ){

	if ( !(GfxBase = (struct GfxBase *) OpenLibrary((UBYTE *) "graphics.library" , 36l ))) {
		WriteStr("\graphics.library\n");
		return FALSE;
		}

	if ( !(DosBase = (struct DosBase *) OpenLibrary((UBYTE *) "dos.library", 36l ))) {
		WriteStr("\tdos.library\n");
		return FALSE;
		}

	if ( !(IntuitionBase = (struct IntuitionBase *) OpenLibrary((UBYTE *) "intuition.library", 36l ))) {
		WriteStr("\tintuition.library\n");
		return FALSE;
		}

	if ( !(AslBase = (struct Library *) OpenLibrary((UBYTE *) "asl.library", 37l ))) {
		WriteStr("\tasl.library\n");
		return FALSE;
		}

	if ( !(GadToolsBase = (struct Library *) OpenLibrary((UBYTE *) "gadtools.library", 36l ))) {
		WriteStr("\tgadtools.library\n");
		return FALSE;
		}

	if ( !(DiskFontBase = (struct Library *) OpenLibrary((UBYTE *) "diskfont.library", 36l ))) {
		WriteStr("\tdiskfont.library\n");
		return FALSE;
		}

	return TRUE;
}




/*
 * --- Close the libraries which are opened by me.
 */
void CloseLibraries( void ){
	if (DiskFontBase)	CloseLibrary( (struct Library *) DiskFontBase );
	if (GadToolsBase)	CloseLibrary( (struct Library *) GadToolsBase );
	if (AslBase)		CloseLibrary( (struct Library *) AslBase );
	if (IntuitionBase)	CloseLibrary( (struct Library *) IntuitionBase );
	if (DosBase)		CloseLibrary( (struct Library *) DosBase );
	if (GfxBase)		CloseLibrary( (struct Library *) GfxBase );
}




/* not really neccesary at the moment */
void LoadIFFColors( void )
{
		;
}


void SaveIFFColors( void )
{
		;
}



long OpenDisplay( void ){
    WORD                 reg = 0, r, offy;

    ReqTags[7].ti_Data = (ULONG)sp_Title;
    ReqTags[9].ti_Data = (Tag)MainScreen;

	if ( ! (thinpazfont8  = OpenDiskFont( &thinpaz8 )) ) {
			;
		}

	if ( version > 38 )
	    for ( r = 0; r < ( 1L << MainScreen->BitMap.Depth ); r++ )
	        GetRGB32( MainScreen->ViewPort.ColorMap, (long)r, 1, &sp_Orig32[r][0] );
	else 
	    for ( r = 0; r < ( 1L << MainScreen->BitMap.Depth ); r++ )
	        sp_Orig[r] = GetRGB4( MainScreen->ViewPort.ColorMap, (long)r );

	if ( ! ( MainVisualInfo = GetVisualInfo( MainScreen, TAG_DONE )))
		return( 2L );

    if ( g = CreateContext( &sp_GList )) {

        ng.ng_LeftEdge      =   79;
        ng.ng_TopEdge       =   3;
        ng.ng_Width         =   216;
        ng.ng_Height        =   10;
        ng.ng_GadgetText    =   "Red:      ";
        ng.ng_TextAttr      =   &thinpaz8;
        ng.ng_GadgetID      =   GD_RED;
        ng.ng_Flags         =   PLACETEXT_LEFT | NG_HIGHLABEL;
        ng.ng_VisualInfo    =   MainVisualInfo;
        ng.ng_UserData      =   NULL;

        g = CreateGadgetA( SLIDER_KIND, g, &ng, sp_PTags );

        sp_Red = g;

        ng.ng_TopEdge       =   17;
        ng.ng_GadgetText    =   "Green:    ";
        ng.ng_GadgetID      =   GD_GREEN;

        g = CreateGadgetA( SLIDER_KIND, g, &ng, sp_PTags );

        sp_Green = g;

        ng.ng_TopEdge       =  31;
        ng.ng_GadgetText    =  "Blue:     ";
        ng.ng_GadgetID      =  GD_BLUE;

        g = CreateGadgetA( SLIDER_KIND, g, &ng, sp_PTags);

        sp_Blue  = g;

        ng.ng_LeftEdge      =  7;
        ng.ng_TopEdge       =  45;
        ng.ng_Width         =  290;
        ng.ng_Height        =  30;
        ng.ng_GadgetText    =  0L;
        ng.ng_GadgetID      =  GD_PALETTE;
        ng.ng_Flags         =  0;

        g = CreateGadget( PALETTE_KIND, g, &ng, GTPA_Depth, (Tag)MainScreen->BitMap.Depth, GTPA_IndicatorWidth, 64l, GTPA_Color, (Tag)reg, TAG_DONE );

        ng.ng_TopEdge       =   79;
        ng.ng_Width         =   56;
        ng.ng_Height        =   14;
        ng.ng_GadgetText    =   "_OK";
        ng.ng_GadgetID      =   GD_OK;
        ng.ng_Flags         =   PLACETEXT_IN;

        g = CreateGadget( BUTTON_KIND, g, &ng, GT_Underscore, '_', TAG_DONE );

        ng.ng_LeftEdge      =   65;
        ng.ng_GadgetText    =   "_Load";
        ng.ng_GadgetID      =   GD_LOAD;

        g = CreateGadget( BUTTON_KIND, g, &ng, GT_Underscore, (Tag)'_', GA_Disabled,TRUE,TAG_DONE );

        ng.ng_LeftEdge      =   123;
        ng.ng_GadgetText    =   "_Save";
        ng.ng_GadgetID      =   GD_SAVE;

        g = CreateGadget( BUTTON_KIND, g, &ng, GT_Underscore, (Tag)'_', GA_Disabled,TRUE,TAG_DONE );

        ng.ng_LeftEdge      =   181;
        ng.ng_GadgetText    =   "_Reset";
        ng.ng_GadgetID      =   GD_RESET;
        ng.ng_Flags         =  0;

        g = CreateGadget( BUTTON_KIND, g, &ng, GT_Underscore, (Tag)'_', TAG_DONE );

        ng.ng_LeftEdge      =   240;
        ng.ng_GadgetText    =   "_CANCEL";
        ng.ng_GadgetID      =   GD_CANCEL;

        g = CreateGadget( BUTTON_KIND, g, &ng, GT_Underscore, (Tag)'_', TAG_DONE );

        ng.ng_TopEdge       =   95;
        ng.ng_LeftEdge      =   65;
        ng.ng_GadgetText    =   "_Jump";
        ng.ng_GadgetID      =   GD_JUMP;

        g = CreateGadget( BUTTON_KIND, g, &ng, GT_Underscore, (Tag)'_', TAG_DONE );
      
        ng.ng_LeftEdge      =   123;
        ng.ng_GadgetText    =   "Cop_y";
        ng.ng_GadgetID      =   GD_COPY;

        g = CreateGadget( BUTTON_KIND, g, &ng, GT_Underscore, (Tag)'_', TAG_DONE );

        ng.ng_LeftEdge      =   181;
        ng.ng_GadgetText    =   "_Exchange";
        ng.ng_GadgetID      =   GD_EXCHANGE;

        g = CreateGadget( BUTTON_KIND, g, &ng, GT_Underscore, (Tag)'_', TAG_DONE );

        ng.ng_LeftEdge      =   240;
        ng.ng_GadgetText    =   "S_pread";
        ng.ng_GadgetID      =   GD_SPREAD;

        g = CreateGadget( BUTTON_KIND, g, &ng, GT_Underscore, (Tag)'_', TAG_DONE );


        if ( g ) {

            ReqTags[6].ti_Data    =   (Tag)sp_GList;

			offy = MainScreen->WBorTop + MainScreen->RastPort.TxHeight + 1;

			ReqTags[3].ti_Data = 116+offy;

            if ( sp_Wnd = OpenWindowTagList( NULL, ReqTags )) {
                GT_RefreshWindow( sp_Wnd, NULL );
                SetProp( reg );
			}
			else
				return NULL;
		}
	}
	return 1;
}





void CloseDisplay( void ) {

	if ( thinpazfont8 )
		CloseFont( thinpazfont8 );

	if ( MainVisualInfo ) {
		FreeVisualInfo( MainVisualInfo );
		MainVisualInfo = NULL;
	}
    if ( sp_Wnd )           CloseWindow( sp_Wnd );
    if ( sp_GList )         FreeGadgets( sp_GList );
}





// ----------------------------------------------------------------------------------------------

/*
 * --- Open the palette editor and wait for user input.
 */
int main( int argc,char *argv[] )
{
    BOOL                 running  = TRUE;
    WORD                 reg = 0;


	OpenLibraries();

	version=IntuitionBase->LibNode.lib_Version;

	MainScreen = (struct Screen *)(IntuitionBase->ActiveScreen);

	if ( OpenDisplay() )
        do {
            WaitPort( sp_Wnd->UserPort );

            while ( ReadIMsg( sp_Wnd )) {

                switch ( IClass ) {

					case	IDCMP_ACTIVEWINDOW:
						ClearMsgPort( sp_Wnd->UserPort );
						break;

                    case    IDCMP_REFRESHWINDOW:
                        GT_BeginRefresh( sp_Wnd );
                        GT_EndRefresh( sp_Wnd, TRUE );
                        break;

                    case    IDCMP_CLOSEWINDOW:
                        goto Cancel;
                        break;

                    case    IDCMP_VANILLAKEY:

                        switch ( Code ) {

                            case    'o':
                                goto Ok;

                            case    'r':
                                goto Reset;

                            case    'c':
                                goto Cancel;

                            case    'j':
                                goto Jump;

                            case    'p':
                                goto Spread;

                            case    'y':
                                goto Copy;

                            case    'e':
                                goto Exchange;
                                
                            case    's':
                                Save:
                                SaveIFFColors();
                                break;

                            case    'l':
                                Load:
                                LoadIFFColors();
                                SetProp( reg );
                                break;
                        }
                        break;

                    case    IDCMP_MOUSEMOVE:

                        switch ( IObject->GadgetID ) {

                            case    GD_RED:
                                sp_Rc = Code;
                                goto Set;

                            case    GD_GREEN:
                                sp_Gc = Code;
                                goto Set;

                            case    GD_BLUE:
                                sp_Bc = Code;

                                Set:
								if ( version>38 )
                                    SetRGB32( &MainScreen->ViewPort, reg,
										 GUN8TO32(sp_Rc), GUN8TO32(sp_Gc), GUN8TO32(sp_Bc) );
								else
                                    SetRGB4( &MainScreen->ViewPort, reg,
										sp_Rc>>4, sp_Gc>>4, sp_Bc>>4 );
                                break;
                        }
                        break;

                    case    IDCMP_GADGETUP:

                            switch ( IObject->GadgetID ) {

                                case    GD_PALETTE:
                                   
                                    reg = Code;
                                    switch ( Edit_Mode ) {
                                        case    EM_COPY:
                                            Edit_Mode = 0;
                                    	    if ( version > 38) {
	                                            GetRGB32(  MainScreen->ViewPort.ColorMap, (long)Prev_Code, 1, &GunsTemp1[0] );
	                                            SetRGB32( &MainScreen->ViewPort, (long) reg ,
				                                    GunsTemp1[0],
				                                    GunsTemp1[1],
				                                    GunsTemp1[2]);	                                                                                   
                                            }
	                                        else {
                                    	        Col12 = GetRGB4( MainScreen->ViewPort.ColorMap, Prev_Code );
                                       	        r4 = (( Col12 >> 8 ) & 0x0f )<<4;
                                    	        g4 = (( Col12 >> 4 ) & 0x0f )<<4;
                                    	        b4 = ( Col12 & 0x0f )<<4;
	                                            SetRGB4( &MainScreen->ViewPort, (long) reg, r4>>4, g4>>4, b4>>4);
                                            }
											break;
                                            
                                        case    EM_EXCHANGE:
                                            Edit_Mode = 0;
                                    	    if ( version > 38) {
	                                            GetRGB32(  MainScreen->ViewPort.ColorMap, (long)Prev_Code, 1, &GunsTemp1[0] );
	                                            GetRGB32(  MainScreen->ViewPort.ColorMap, (long)reg, 1, &GunsTemp2[0] );
	                                            SetRGB32( &MainScreen->ViewPort, (long) reg ,
				                                    GunsTemp1[0],
				                                    GunsTemp1[1],
				                                    GunsTemp1[2]);
	                                            SetRGB32( &MainScreen->ViewPort, (long) Prev_Code ,
				                                    GunsTemp2[0],
				                                    GunsTemp2[1],
				                                    GunsTemp2[2]);
                                            }
	                                        else {
                                    	        Col12 = GetRGB4( MainScreen->ViewPort.ColorMap, Prev_Code );
                                       	        r4 = (( Col12 >> 8 ) & 0x0f )<<4;
                                    	        g4 = (( Col12 >> 4 ) & 0x0f )<<4;
                                    	        b4 = ( Col12 & 0x0f )<<4;
	                                        
                                    	        Col12_2 = GetRGB4( MainScreen->ViewPort.ColorMap, reg );
                                       	        r4_2 = (( Col12_2 >> 8 ) & 0x0f )<<4;
                                    	        g4_2 = (( Col12_2 >> 4 ) & 0x0f )<<4;
                                    	        b4_2 = ( Col12_2 & 0x0f )<<4;
	                                            
                                                SetRGB4( &MainScreen->ViewPort, (long) reg, r4>>4, g4>>4, b4>>4);
                                                SetRGB4( &MainScreen->ViewPort, (long) Prev_Code, r4_2>>4, g4_2>>4, b4_2>>4);
                                            }
											break;

                                        case    EM_SPREAD:
                                            Edit_Mode = 0;
                                    	    if ( version > 38) {
												LONG	red,grn,blu,redi,grni,blui,lp,pen_num;

												// previous pen
	                                            GetRGB32(  MainScreen->ViewPort.ColorMap, (long)Prev_Code, 1, &GunsTemp1[0] );
												// current pen
	                                            GetRGB32(  MainScreen->ViewPort.ColorMap, (long)reg, 1, &GunsTemp2[0] );

												red = GunsTemp1[0]&0xff000000 >>16;
												grn = GunsTemp1[1]&0xff000000 >>16;
												blu = GunsTemp1[2]&0xff000000 >>16;
												redi = GunsTemp2[0]&0xff000000 >>16;
												grni = GunsTemp2[1]&0xff000000 >>16;
												blui = GunsTemp2[2]&0xff000000 >>16;

												if( Prev_Code < reg ){		// XX------>
													lp= reg-Prev_Code;
													redi = (redi-red)/lp;
													grni = (grni-grn)/lp;
													blui = (blui-blu)/lp;
													pen_num = Prev_Code;
												}
												else
												if( reg <= Prev_Code ){		//<-------XX
													lp= Prev_Code-reg;
													redi = (red-redi)/lp;
													grni = (grn-grni)/lp;
													blui = (blu-blui)/lp;
													pen_num = reg;
													red = GunsTemp2[0]&0xff000000 >>16;
													grn = GunsTemp2[1]&0xff000000 >>16;
													blu = GunsTemp2[2]&0xff000000 >>16;
												}
												//do the spread  24bit
												while(lp-- > 0){
		                                            SetRGB32( &MainScreen->ViewPort, (long) pen_num++ ,
					                                    red<<16, grn<<16, blu<<16 );
													red+=redi;grn+=grni;blu+=blui;
												}

                                            }
	                                        else {
												LONG	red,grn,blu,redi,grni,blui,lp,pen_num;

                                    	        Col12 = GetRGB4( MainScreen->ViewPort.ColorMap, Prev_Code );
                                    	        Col12_2 = GetRGB4( MainScreen->ViewPort.ColorMap, reg );

                                       	        red = (( Col12 >> 8 ) & 0x0f )<<4;
                                    	        grn = (( Col12 >> 4 ) & 0x0f )<<4;
                                    	        blu = ( Col12 & 0x0f )<<4;
                                       	        redi = (( Col12_2 >> 8 ) & 0x0f )<<4;
                                    	        grni = (( Col12_2 >> 4 ) & 0x0f )<<4;
                                    	        blui = ( Col12_2 & 0x0f )<<4;
	                                            
												if( Prev_Code < reg ){		// XX------>
													lp= reg-Prev_Code;
													redi = (redi-red)/lp;
													grni = (grni-grn)/lp;
													blui = (blui-blu)/lp;
													pen_num = Prev_Code;
												}
												else
												if( reg <= Prev_Code ){		//<-------XX
													lp= Prev_Code-reg;
													redi = (red-redi)/lp;
													grni = (grn-grni)/lp;
													blui = (blu-blui)/lp;
													pen_num = reg;red=redi;grn=grni;blu=blui;
												}
												//do the spread  12bit
												while(lp-- > 0){
	                                                SetRGB4( &MainScreen->ViewPort, (long) pen_num++, red>>4, grn>>4, blu>>4);
													red+=redi;grn+=grni;blu+=blui;
												}
                                            }
											break;                                             
                                            
                                    }    
                                    SetProp( (long) reg);
                                    Prev_Code = Code;    
                                    break;    
                                case    GD_OK:
                                    Ok:
                                    running = FALSE;
                                    break;

                                case    GD_RESET:
                                    Reset:
									loadOrig();
                                    SetProp( reg );
                                    break;

                                case    GD_CANCEL:
                                    Cancel:
									loadOrig();
                                    running = FALSE;
                                    break;

                                case    GD_SAVE:
                                    goto Save;

                                case    GD_LOAD:
                                    goto Load;
				    
                                case    GD_JUMP:
				                    Jump:
								    MainScreen = (struct Screen *)(MainScreen->NextScreen);
								    if ( !MainScreen )
								    	MainScreen = (struct Screen *)(IntuitionBase->FirstScreen);
								    CloseDisplay();
							        OpenDisplay();
								    ScreenToFront( MainScreen );
								    break;                      
                                case    GD_COPY:
                                    Copy:              
                                    Edit_Mode = EM_COPY;
                                    break;
                                    
                                case    GD_EXCHANGE :
                                    Exchange:
                                    Edit_Mode = EM_EXCHANGE;
                                    break;
                                    
                                case    GD_SPREAD :
                                    Spread:
                                    Edit_Mode = EM_SPREAD;
                                    break;
                                    
                            }
                            break;
                        }
                    }
           } while ( running );


	CloseDisplay();

	CloseLibraries();

    Exit( TRUE );
}

