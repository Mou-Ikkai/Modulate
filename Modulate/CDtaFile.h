#pragma once

#include <deque>
#include <list>
#include <string>

#include "Error.h"
#include "Utils.h"

enum eNodeType {
    ENodeType_Integer0 = 0,
    ENodeType_Float = 1,
    ENodeType_String = 5,
    ENodeType_Integer6 = 6,
    ENodeType_Integer8 = 8,
    ENodeType_Integer9 = 9,
    ENodeType_Tree1 = 16,
    ENodeType_Tree2 = 17,
    ENodeType_Id = 18,
    ENodeType_IncludeFile = 33,
    ENodeType_Define = 35,
    ENodeType_Invalid
};

struct SSongConfig
{
    std::string mId = "";
    std::string mName = "";
    std::string mUnlockMethod = "";
    std::string mType = "";
    std::string mPath = "";
    std::string mArena = "";
    int miUnlockCount = -1;
};

class CDtaNodeBase
{
public:
    CDtaNodeBase( CDtaNodeBase* lpParent, int liNodeId ) : mpParent( lpParent ), msNodeId( liNodeId ) {}

    virtual ~CDtaNodeBase()
    {
        for( auto& lpNode : maChildren )
        {
            delete lpNode;
        }
    }

    void SetId( int liNewId )
    {
        msNodeId = liNewId;
    }

    bool IsBaseNode() const
    {
        return mbIsBaseNode;
    }

    void SetTypeOverride( int liNewType )
    {
        miTypeOverride = liNewType;
    }

    CDtaNodeBase* AddNode( short lsNodeId )
    {
        maChildren.push_back( new CDtaNodeBase( this, lsNodeId ) );
        return maChildren.back();
    }

    template < class T >
    CDtaNodeBase* AddChild( const T& lValue );

    CDtaNodeBase* AddChild( CDtaNodeBase* lpValue )
    {
        maChildren.push_back( lpValue );
        return lpValue;
    }
    
    CDtaNodeBase* AddChildToFront( CDtaNodeBase* lpValue )
    {
        maChildren.insert( maChildren.begin() + 1, lpValue );
        return lpValue;
    }

    const std::deque< CDtaNodeBase* >&
    GetChildren() const
    {
        return maChildren;
    }

    CDtaNodeBase* GetParent() const
    {
        return mpParent;
    }

    CDtaNodeBase* FindNode( const std::string& lName ) const;

    void Save( unsigned char*& lpStream ) const;

    virtual void SaveToStream( unsigned char*& lpStream ) const;

private:
    short GetHighestNodeId() const
    {
        if( maChildren.empty() )
        {
            return msNodeId;
        }

        return maChildren.back()->GetHighestNodeId();
    }

    std::deque< CDtaNodeBase* > maChildren;

protected:
    CDtaNodeBase* mpParent;

    int   miTypeOverride = -1;
    short msNodeId;
    bool  mbIsBaseNode = true;
};


template < class T >
class CDtaNode : public CDtaNodeBase
{
public:
    CDtaNode( CDtaNodeBase* lpParent, int liNodeId, const T& lValue )
        : CDtaNodeBase( lpParent, liNodeId )
    {
        mValue = lValue;
        mbIsBaseNode = false;
    }

    void SaveToStream( unsigned char*& lpStream ) const override { }

    const T& GetValue() const
    {
        return mValue;
    }

    void SetValue( const T& lValue )
    {
        mValue = lValue;
    }

private:
    T mValue;
};

template < class T >
CDtaNodeBase* CDtaNodeBase::AddChild( const T& lValue )
{
    CDtaNode< T >* lNewNode = new CDtaNode< T >( this, msNodeId, lValue );
    maChildren.push_back( lNewNode );
    return lNewNode;
}

void CDtaNode< int >::SaveToStream( unsigned char*& lpStream ) const
{
    WriteToStream< int >( lpStream, miTypeOverride == -1 ? ENodeType_Integer0 : miTypeOverride );
    WriteToStream< int >( lpStream, mValue );
}

void CDtaNode< unsigned int >::SaveToStream( unsigned char*& lpStream ) const
{
    WriteToStream< int >( lpStream, miTypeOverride == -1 ? ENodeType_Integer0 : miTypeOverride );
    WriteToStream< int >( lpStream, (int)mValue );
}

void CDtaNode< float >::SaveToStream( unsigned char*& lpStream ) const
{
    WriteToStream< int >( lpStream, miTypeOverride == -1 ? ENodeType_Float : miTypeOverride );
    WriteToStream< float >( lpStream, mValue );
}

void CDtaNode< std::string >::SaveToStream( unsigned char*& lpStream ) const
{
    WriteToStream< int >( lpStream, miTypeOverride == -1 ? ENodeType_String : miTypeOverride );
    WriteToStream( lpStream, mValue );
}


class CDtaFile
{
public:
    CDtaFile() : mRootNode( nullptr, 1 ) {}

    eError Load( const char* lpFilename );
    eError Save( const char* lpFilename ) const;

    std::vector< SSongConfig > GetSongs() const;
    eError SetSongs( const std::vector< SSongConfig >& laSongs );

    void GetSongData( std::vector< SSongConfig >& laSongs ) const;
    eError UpdateSongData( const std::vector< SSongConfig >& laSongs );

private:
    eError AddTreeNode( CDtaNodeBase* lpParent, const char*& lpData, eNodeType leNodeType );

    CDtaNodeBase mRootNode;
};


class CMoggsong
{
public:
    eError LoadMoggSong( const char* lpFilename );
    eError Save( const char* lpFilename, bool lbDoOutput = true ) const;

    void SetMoggPath( const std::string& lFilename ) { mMoggPath = lFilename; }
    void SetMidiPath( const std::string& lFilename ) { mMidiPath = lFilename; }

    float GetBPM() const { return mfBPM; }
    const std::string& GetTitle() const { return mTitle; }
    const std::string& GetArenaName() const { return mArenaName; }

private:
    eError ProcessMoggSongKey( char*& lpData, __int64 liDataSize );

    bool mbLoadingTrackData = false; // TODO - move to moggsongloader class

    std::string mMoggPath = "";
    std::string mMidiPath = "";

    std::string mTitle = "";
    std::string mTitleShort = "";
    std::string mArtist = "";
    std::string mArtistShort = "";
    std::string mDescription = "";
    std::string mUnlockRequirement = "";

    float mfTunnelScale = 0.0f;

    float mfBPM = 0.0f;
    int miLength = 0;
    int miCountIn = 0;
    int miBossLevel = -1;

    int miPreviewStartMS = 0;
    int miPreviewDurationMS = 10000;

    struct sTrackDefinition
    {
        std::string mName;
        std::vector< int > maChannels;
        std::vector< float > maPans;
        std::vector< float > maVolumes;
        std::string mEventString;
    };

    std::vector< sTrackDefinition > maTracks;
    std::vector< int > maEnableOrder;
    std::vector< int > maSectionStartBars;
    std::vector< float > maActiveTrackDB;

    std::string mArenaPath = "";
    std::string mArenaName = "";
    
    enum eDifficulty {
        eDifficulty_Beginner,
        eDifficulty_Intermediate,
        eDifficulty_Expert,
        eDifficulty_Super,
        eDifficulty_NumTypes
    };
    enum eStars {
        eStars_2,
        eStars_3,
        eStars_G,
        eStars_NumTypes
    };
    int maScoreGoals[ eDifficulty_NumTypes ][ eStars_NumTypes ] = { { 0, 20, 30 }, { 0, 20, 30 }, { 0, 20, 30 }, { 0, 20, 30 } };
};

