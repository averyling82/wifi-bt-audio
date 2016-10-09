#ifndef __ID3_H__
#define __ID3_H__

#include "AddrSaveMacro.h"

#define FSEEK_SET   0
#define FSEEK_CUR   1
#define FSEEK_END   2

#define ID3_PICTUTER_WIDTH     28// 25
#define ID_3PICTUTER_HEIGTH    28//25

#define MAX_ID3V1_SIZE  128

#define MAX_ID3V2_FIELD  (SYS_SUPPROT_STRING_MAX_LEN * 2 + 10)

#define MAX_ID3V2_YEAR  40

#define MAX_ID3_FIND_SIZE 512//1024//2048
#define MAX_WMA_TAG_SIZE  128

#define ID3_FLAG_UNSYNCH   0x80 // a. Unsynchronisation
#define ID3_FLAG_EXTHEADER 0x40 // b. Extended Header
#define ID3_FLAG_INDICATOR 0x20 // c. Experimental Indicator
#define ID3_FLAG_FOOTER    0x10 // d. Footer Present

#define ID3_FLAG_UPDATE    0x40
#define ID3_FLAG_CRCDATA   0x20
#define ID3_FLAG_RESTRICT  0x10

#define ID3_ENCODE_MASK    0x03
#define ID3_ENCODE_ISO88591   0x00
#define ID3_ENCODE_UTF16   0x01
#define ID3_ENCODE_UTF16BE   0x02
#define ID3_ENCODE_UTF8    0x03

#define ID3_UNICODE_BOM1   0xFFFE
#define ID3_UNICODE_BOM2   0xFEFF

#define ID3_GENRE_MAX_CNT   192

#define WAVE_HEAD_SIZE     90

// MP3 Header Parse
/////////////////////////////////////////////////////////////////
enum ID3Version
{
    ID3_V10,
    ID3_V11,
    ID3_V2X
};

enum ID3FrameInfo
{
    ID3_TITLE,
    ID3_ARTIST,
    ID3_GENRE,
    ID3_ALBUM,
    ID3_YEAR,
    ID3_COMMENT,
    ID3_COMPOSER,
    ID3_ORIGARTIST,
    ID3_COPYRIGHT,
    ID3_URL,
    ID3_ENCODEDBY,
    ID3_NONINFO,
    ID3_TRACK,
    ID3_PICTURE,
    ID3_GEOB,
    ID3_NOTUSEDFRAME,
    ID3_MCDI
};

typedef struct
{
    short Title[MEDIA_ID3_SAVE_CHAR_NUM];
    short Album[MEDIA_ID3_SAVE_CHAR_NUM];
    short Author[MEDIA_ID3_SAVE_CHAR_NUM];
    short Genre[MEDIA_ID3_SAVE_CHAR_NUM];
    short Track[MEDIA_ID3_SAVE_CHAR_NUM];
} ID3;

typedef __packed struct stID3Info
{
    unsigned char mTAG[3];
    unsigned char mTitle[30];
    unsigned char mArtist[30];
    unsigned char mAlbum[30];
    unsigned char mYear[4];
    unsigned char mComment[29];
    unsigned char mTrack;
    unsigned char mGenre;
}stID3InfoType, *pstID3InfoType;

/*
typedef  PACKED struct stID3V2XInfo
{
    unsigned short mTrack;
    unsigned char  mYear[MAX_ID3V2_YEAR];
    unsigned char  mMetaData[ID3_NONINFO-1][MAX_ID3V2_FIELD];
}stID3V2XInfoType, *pstID3V2XInfoType;
*/


typedef __packed struct _ID3V2X_INFO
{
    unsigned short mTrack[4];
    unsigned short nTrack[4];
    unsigned short mYear[MAX_ID3V2_YEAR];
    unsigned short  id3_title[MAX_ID3V2_FIELD / 2];
    unsigned short  id3_singer[MAX_ID3V2_FIELD / 2];
    unsigned short  id3_album[MAX_ID3V2_FIELD / 2];
    unsigned short  id3_genre[MAX_ID3V2_FIELD / 2];
}
ID3V2X_INFO;

typedef __packed struct _ID3V2XHeaderType
{
    unsigned char  mID3[3];
    unsigned short mVersion;
    unsigned char  mFlags;
    unsigned char  mSize[4];

}ID3V2XHeaderType, *pID3V2XHeaderType;

typedef  __packed  struct _ID3V2XFooterType
{
    unsigned char  mID3[3];
    unsigned short mVersion;
    unsigned char  mFlags;
    unsigned char  mSize[4];
}ID3V2XFooterType, *pID3V2XFooterType;

typedef __packed struct _ID3V2XExtHeaderType
{
    unsigned char  mHeaderSize[4];
    unsigned char  mFlagNum;
    unsigned char  mExtFlags;
}ID3V2XExtHeaderType, *pID3V2XExtHeaderType;

typedef  __packed struct _ID3V2XFrameInfoType
{
    unsigned char  mFrameID[4];
    unsigned char  mSize[4];
    unsigned char  mFlags[2];
}ID3V2XFrameInfoType, *pID3V2XFrameInfoType;

typedef __packed struct _ID3V22FrameInfoType
{
    unsigned char  mFrameID[3];
    unsigned char  mSize[3];
}ID3V22FrameInfoType, *pID3V22FrameInfoType;

typedef  __packed  struct _ID3V2XEncodeInfoType
{
    unsigned char   mEncodeType;
    unsigned short  mUnicodeBOM;
}ID3V2XEncodeInfoType, *pID3V2XEncodeInfoType;

// WMA Header Parse
/////////////////////////////////////////////////////////////////
struct _WMATag
{
    unsigned int tagexist;
    unsigned char artist[MAX_WMA_TAG_SIZE+2];
    unsigned char title[MAX_WMA_TAG_SIZE+2];
};

typedef __packed struct _ASFHeaderObjectType
{
    unsigned char  mObjectID[16];
    unsigned int   mObjectSize[2];
    unsigned int   mNumOfHeadObj;
    unsigned char  mReserved[2];
}ASFHeaderObjectType, *pASFHeaderObjectType;

typedef __packed struct _ASFFilePropertiesObjectType
{
    unsigned char  mObjectID[16];
    unsigned int   mObjectSize[2];
    unsigned char  mFileID[16];
    unsigned int   mFileSize[2];
    unsigned char  mCreationDate[8];
    unsigned char  mDataPacketsCnt[8];
    unsigned int   mPlayDuration[2];
    unsigned int   mSendDuration[2];
    unsigned char  mPreroll[8];
    unsigned int   mFlags;
    unsigned int   mMinDataPacketSize;
    unsigned int   mMaxDataPacketSize;
    unsigned int   mMaxBitrate;
}ASFFilePropertiesObjectType, *pASFFilePropertiesObjectType;

typedef __packed struct _ASFExtContentDescripObjType
{
    unsigned char  mObjectID[16];
    unsigned int   mObjectSize[2];
    unsigned short mContentDescriptorsCnt;
}ASFExtContentDescripObjType, *pASFExtContentDescripObjType;

typedef __packed struct _WMAMetaHeaderType
{
    unsigned char  mObjectID[16];
    unsigned int   mObjectSize[2];
    unsigned short cbCDTitle;
    unsigned short cbCDAuthor;
    unsigned short cbCDCopyright;
    unsigned short cbCDDescription;
    unsigned short cbCDRating;
}WMAMetaHeaderType, *pWMAMetaHeaderType;

typedef __packed struct _ASFContentDescriptorType
{
    unsigned char  mSize;
    unsigned char  mID;
    unsigned char* mDescriptor;
}ASFContentDescriptorType, *pASFContentDescriptorType;


#ifdef OGG_HEADER_PARSING
typedef  struct _OGGContentDescriptorType
{
    unsigned char  mSize;
    unsigned char  mID;
    unsigned char* Big_mDescriptor;
    unsigned char* Small_mDescriptor;
}OGGContentDescriptorType, *pOGGContentDescriptorType;

#endif

enum
{
    WMA_DESCRIPTOR_UNKNOWN,
    WMA_DESCRIPTOR_ID3TAG,
    WMA_DESCRIPTOR_ALBUM,
    WMA_DESCRIPTOR_GENRE,
    WMA_DESCRIPTOR_TRACK,
    WMA_DESCRIPTOR_TRACKNUMBER,
    WMA_DESCRIPTOR_YEAR,
#ifdef RK_JPEG_ASF
    WMA_DESCRIPTOR_PICTURE,
#endif
    WMA_DESCRIPTOR_MAX
};
// WAV Header Parse
/////////////////////////////////////////////////////////////////
typedef  struct _WAVMetaHeaderType
{
    unsigned char  mRIFF[4];    // "RIFF"
    unsigned int mFileSize;    //
    unsigned char  mWAVSignature[4];  //
    unsigned char  mfmt[3];    // "fmt"
    unsigned char  mTemp1[5];    // X + 50 + XXX
    unsigned char mFormtTag[2];   //
    unsigned short  mChannel;    //
    unsigned int mSamplePerSec;   //
    unsigned int  mAvgBytesPerSec;  //
    unsigned short  mBlockAlign;   //
    unsigned short  mBitPerSample;      //
    unsigned short  mcbSize;    // Extended information size after header
    unsigned short  mSamplePerBlock;   //
    unsigned char   mwNumCoef[2];   //
    unsigned char   maCoef[7][4];   //
    unsigned char   mfact[4];    // "fact"
    unsigned char  mTemp2[4];    // XXXX
    unsigned int    mDuration;    //
    unsigned char  mdata[4];    // "data"
    unsigned char  mTemp3[4];    // XXXX
}WAVMetaHeaderType, *pWAVMetaHeaderType;

/*****************************************************************************************/
/* APE tags */
#define APE_TAG_VERSION               2000
#define APE_TAG_FOOTER_BYTES          32
#define APE_TAG_FLAG_CONTAINS_HEADER  (1 << 31)
#define APE_TAG_FLAG_IS_HEADER        (1 << 29)

//Standard APE tag fields
#define APE_TAG_FIELD_TITLE                     "Title"
#define APE_TAG_FIELD_ARTIST                    "Artist"
#define APE_TAG_FIELD_ALBUM                     "Album"
#define APE_TAG_FIELD_GENRE                     "Genre"
#define APE_TAG_FIELD_TRACK                     "Track"
#define APE_TAG_FIELD_COVER_ART_FRONT           "Cover Art (front)"

/* OGG tags */
#define STREAM_STRUCTURE_VERSION        0   //表示当前Ogg文件格式的版本,目前为0
#define STREAM_CONTINUS_PAGE            1   //页包含的媒体编码数据于前一页同属于一个逻辑流的同一packet; 若未设置, 本页是一个新的packet
#define STREAM_FIRST_PAGE               2   //表示逻辑流的第一个页bos; 未设, 不是第一个页
#define STREAM_LAST_PAGE                4   //表示逻辑流的最后一页eos; 未设, 不是第一个页
#define STREAM_PACKET_NOT_END           -1  //到此页为止, 逻辑流的packet还未结束

#define MAX_NUM_PAGE_SEGMENTS           255
#define MAX_PAGE_SIZE                   (27+255+255*255)
#define PAGE_HEADER_NOT_SEGMENTS_BYTES  27


/* FLAC tag */
#define METADATA_TYPE_STREAMINFO        0
#define METADATA_TYPE_PADDING           1
#define METADATA_TYPE_APPLICATION       2
#define METADATA_TYPE_SEEKTABLE         3
#define METADATA_TYPE_VORBIS_COMMENT    4
#define METADATA_TYPE_CUESHEET          5
#define METADATA_TYPE_PICTURE           6
#define METADATA_TYPE_UNDEFINED         7

//Standard Vorbis tag fields
#define VORBIS_TAG_FIELD_TITLE                     "TITLE"
#define VORBIS_TAG_FIELD_ARTIST                    "ARTIST"
#define VORBIS_TAG_FIELD_ALBUM                     "ALBUM"
#define VORBIS_TAG_FIELD_GENRE                     "GENRE"
#define VORBIS_TAG_FIELD_TRACKNUMBER               "TRACKNUMBER"
#define VORBIS_TAG_FIELD_PICTURE                   "METADATA_BLOCK_PICTURE"

typedef enum _TAG_ID
{
    TAGID_NULL          = 0,
    TAGID_TITLE         = 1,
    TAGID_ARTIST        = 2,
    TAGID_ALBUM         = 3,
    TAGID_GENRE         = 4,
    TAGID_TRACK         = 5,
} TAG_ID;
/*****************************************************************************************/

#ifdef OGG_HEADER_PARSING
// OGG Header Parse
/////////////////////////////////////////////////////////////////

enum
{
    OGG_DESCRIPTOR_TITLE,
    OGG_DESCRIPTOR_VERSION ,
    OGG_DESCRIPTOR_ALBUM ,
    OGG_DESCRIPTOR_TRACKNUMBER ,
    OGG_DESCRIPTOR_ARTIST,
    OGG_DESCRIPTOR_PERFORMER,
    OGG_DESCRIPTOR_COPYRIGHT,
    OGG_DESCRIPTOR_LICENSE,
    OGG_DESCRIPTOR_ORGANIZATION ,
    OGG_DESCRIPTOR_DESCRIPTION ,
    OGG_DESCRIPTOR_GENRE ,
    OGG_DESCRIPTOR_DATE,
    OGG_DESCRIPTOR_LOCATION,
    OGG_DESCRIPTOR_CONTACT,
    OGG_DESCRIPTOR_ISRC,
    OGG_DESCRIPTOR_YEAR,

};

typedef struct _OGGHeaderObjectType
{
    unsigned char   CapturePattern[4];
    unsigned char   Version;
    unsigned char   HeaderType;
    unsigned long long GranulePosition;
    unsigned int   BitstreamSerialNumber;
    unsigned int   PageSequenceNumber;
    unsigned int   Checksum;
    unsigned char   PageSegments;
}OGGHeaderObjectType, *pOGGHeaderObjectType;

#endif

//  music album cover data struct
typedef struct _ID3AlbumArtData
{
    unsigned int AlbumArtFlag;      //flag:alumb cover. 1: have，0: no
    unsigned int AlbumArtWidth;     //the width of alumb cover thumbnail.
    unsigned int AlbumArtHigh;      //the hight of alumb cover thumbnail.
    unsigned char AlbumArtBuff[ID3_PICTUTER_WIDTH*ID_3PICTUTER_HEIGTH*2];//the data save pointer of alumb cover,one logic screen memery space,this memery get from dynamic heap.
} ID3_AudioAlbumArt, *pID3_AudioAlbumArt;

#define ID3_GetIntValue(x)   ((x[0]<<21)|(x[1]<<14)|(x[2]<<7)|x[3])
#define ID3_GetShortValue(x)  ((x[0]<<8)|(x[1]))
#define ID3_Get3ByteValue(x)  ((x[0]<<16)|(x[1]<<8)|(x[2]))
#define ID3_Get4byteIntValue(x)   ((x[0]<<24)|(x[1]<<16)|(x[2]<<8)|x[3])

extern unsigned char ID3AlbumArtInit(unsigned int Width, unsigned int High);
extern unsigned char ID3CheckAlbumArtExist(void);
extern void ID3AlbumArtDeInit(void);

extern int ID3_GetID3(int fd);

extern int PROFILE_GetID3MP3Info(int fHandle, unsigned char *tempbuf, void *pID3Info);

extern void     ID3_MakeWideChar2Unicode(char *SourceBuf, char *TargetBuf, int iSourceMaxSize);
#endif


