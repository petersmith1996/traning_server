#include "DataFrame.h"

//////////////////////////
//Message
//////////////////////////

class DATA_PVD{
public:
    MinuteOfTheYear timeStamp;
    ProbeSegmentNumber segNum;
    VehicleIdent probeID;
    FullPositionVector startVector;
    VehicleClassification vehicleType;
    SnapshotList snapshots;
};

class DATA_MAP{
public:
    MinuteOfTheYear timeStamp;
    MsgCount msgIssueRevision;
    LayerType layerType;
    LayerID layerID;
    IntersectionGeometryList intersections;
    RoadSegmentList roadSegments;
    DataParameters dataParameters;
    RestrictionClassList restrictionList;
};

class DATA_BSM
{
public:
    BSMcoreData coreData;
    BSMpartIIExtension partII;
    Reg_BasicSafetyMessage regional;
};

class DATA_SPAT
{
public:
    MinuteOfTheYear timeStamp;
    DescriptiveName name;
    IntersectionStateList intersections;
};