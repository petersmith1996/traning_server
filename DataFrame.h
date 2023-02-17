#include "DataElement.h"

//////////////////////////
//Data Frame
//////////////////////////

class RegulatorySpeedLimit
{
public:
    SpeedLimitType type;
    Velocity speed;
};

typedef RegulatorySpeedLimit* SpeedLimitList; // size 1..9

enum class LaneDataAttribute_Type{
    pathEndPointAngle,
    laneCrownPointCenter,
    laneCrownPointLeft,
    laneCrownPointRight,
    laneAngle,
    speedLimits
};

union LaneDataAttribute_u{
    DeltaAngle pathEndPointAngle;
    RoadwayCrownAngle laneCrownPointCenter;
    RoadwayCrownAngle laneCrownPointLeft;
    RoadwayCrownAngle laneCrownPointRight;
    MergeDivergeNodeAngle laneAngle;
    SpeedLimitList speedLimits;
};

typedef struct LaneDataAttribute{
    LaneDataAttribute_Type type;
    LaneDataAttribute_u val;
}LaneDataAttribute;

class Node_XY_20b{
public:
    Offset_B10 x;
    Offset_B10 y;
};

class Node_XY_22b{
public:
    Offset_B10 x;
    Offset_B10 y;
};

class Node_XY_24b{
public:
    Offset_B10 x;
    Offset_B10 y;
};

class Node_XY_26b{
public:
    Offset_B10 x;
    Offset_B10 y;
};

class Node_XY_28b{
public:
    Offset_B10 x;
    Offset_B10 y;
};

class Node_XY_32b{
public:
    Offset_B10 x;
    Offset_B10 y;
};

class Node_LLmD_64b{
public:
    Longitude lon;
    Latitude lat;
};

enum class offsetXaxis_Type{
    small,
    large
};

enum class offsetYaxis_Type{
    small,
    large
};

typedef union offsetXaxis_u {
    DrivenLineOffsetSm small;
    DrivenLineOffsetLg large;
} offsetXaxis_u;

typedef union offsetYaxis_u {
    DrivenLineOffsetSm small;
    DrivenLineOffsetLg large;
} offsetYaxis_u;

class ComputedLane
{
public:
    LaneID referenceLaneID;
    struct offsetXaxis{
        offsetXaxis_Type type;
        offsetXaxis_u val;
    };
    struct offsetYaxis{
        offsetYaxis_Type type;
        offsetYaxis_u val;
    };
    Angle rotateXY;
    Scale_B12 scaleXaxis;
    Scale_B12 scaleYaxis;
};

typedef NodeAttributeXY* NodeAttributeXYList; // size 1..8

typedef SegmentAttributeXY* SegmentAttributeXYList; // size 1..8

typedef LaneDataAttribute* LaneDataAttributeList; // size 1..8

class NodeAttributeSetXY
{
public:
    NodeAttributeXYList localNode;
    SegmentAttributeXYList disabled;
    SegmentAttributeXYList enabled;
    LaneDataAttributeList data;
    Offset_B10 dWidth;
    Offset_B10 dElevation;
};

enum class NodeOffsetPointXY_Type{
    node_XY1,
    node_XY2,
    node_XY3,
    node_XY4,
    node_XY5,
    node_XY6,
    node_LatLon
};

union NodeOffsetPointXY_u
{
    Node_XY_20b node_XY1;
    Node_XY_22b node_XY2;
    Node_XY_24b node_XY3;
    Node_XY_26b node_XY4;
    Node_XY_28b node_XY5;
    Node_XY_32b node_XY6;
    Node_LLmD_64b node_LatLon;
};

typedef struct NodeOffsetPointXY{
    NodeOffsetPointXY_Type type;
    NodeOffsetPointXY_u val;
}NodeOffsetPointXY;

class NodeXY
{
public:
    NodeOffsetPointXY delta;
    NodeAttributeSetXY attributes;
};

typedef NodeXY* NodeSetXY;

union NodeListXY_u
{
    NodeSetXY nodes;
    ComputedLane computed;
};

enum class NodeListXY_Type{
    nodes,
    computed
};

typedef struct NodeListXY
{
    NodeListXY_Type type;
    NodeListXY_u val;
}NodeListXY;

enum class LaneTypeAttributes_Type{
    vehicle,
    crosswalk,
    bikeLane,
    sidewalk,
    median,
    striping,
    treckedVehicle,
    parking
};

union LaneTypeAttributes_u
{
    LaneAttributes_Vehicle vehicle;
    LaneAttributes_Crosswalk crosswalk;
    LaneAttributes_Bike bikeLane;
    LaneAttributes_Sidewalk sidewalk;
    LaneAttributes_Barrier median;
    LaneAttributes_Striping striping;
    LaneAttributes_TrackedVehicle treckedVehicle;
    LaneAttributes_Parking parking;
};

typedef struct LaneTypeAttributes{
    LaneTypeAttributes_Type type;
    LaneTypeAttributes_u val;
}LaneTypeAttributes;

class LaneAttributes
{
public:
    LaneDirection directionalUse;
    LaneSharing sharedWith;
    LaneTypeAttributes laneType;
    //struct LaneTypeAttributes{

    //}
};

class ConnectingLane{
public:
    LaneID lane;
    AllowedManeuvers maneuvers;
};

class IntersectionReferenceID
{
public:
    RoadRegulatorID region;
    IntersectionID id;
};

class Connection{
public:
    ConnectingLane connectingLane;
    IntersectionReferenceID remoteIntersection;
    SignalGroupID signalGroup;
    RestrictionClassID userClass;
    LaneConnectionID connectionID;
};

typedef Connection* ConnectsToList; // size 1..16

typedef LaneID* OverlayLaneList; // size 1..5

class GenericLane
{
public:
    LaneID laneID;
    DescriptiveName name;
    ApproachID ingressApproach;
    ApproachID egressApproach;
    LaneAttributes laneAttributes;
    AllowedManeuvers maneuvers;
    NodeListXY nodeList;
    ConnectsToList connectsTo;
    OverlayLaneList overlays;
};

typedef GenericLane* LaneList;

class Position3D
{
public:
    Latitude lat;
    Longitude Long;
    Elevation elevation;
};

class SignalControlZone{
public:

};

typedef SignalControlZone* PreemptPriorityList;

enum class RoadAuthorityID_Type{
    fullRdAuthID,
    relRdAuthID
};

union RoadAuthorityID_u{
    FullRoadAuthorityID fullRdAuthID;
    RelativeRoadAuthorityID relRdAuthID;
};

typedef struct RoadAuthorityID{
    RoadAuthorityID_Type type;
    RoadAuthorityID_u val;
}RoadAuthorityID;

class IntersectionGeometry
{
public:
    DescriptiveName name;
    IntersectionReferenceID id;
    MsgCount revision;
    Position3D refPoint;
    LaneWidth laneWidth;
    SpeedLimitList speedLimits;
    LaneList laneSet;
    PreemptPriorityList preemptPriorityData;
    RoadAuthorityID roadAuthorityID;
};

typedef IntersectionGeometry* IntersectionGeometryList; // size 1..32


class RoadSegmentReferenceID{
public:
    RoadRegulatorID region;
    RoadSegmentID id;
};

class RoadSegment{
public:
    DescriptiveName name;
    RoadSegmentReferenceID id;
    MsgCount revision;
    Position3D refPoint;
    LaneWidth laneWidth;
    SpeedLimitList speedLimits;
    RoadLaneSetList roadLaneSet;
};

typedef RoadSegment* RoadSegmentList; // size 1..32

class DataParameters{
public:
    char processMethod[255];
    char processAgency[255];
    char lastCheckedDate[255];
    char geoidUsed[255];
};

class RestrictionUserType
{
public:
    RestrictionAppliesTo basicType;
};

typedef RestrictionUserType* RestrictionUserTypeList;

class RestrictionClassAssignment
{
public:
    RestrictionClassID id;
    RestrictionUserTypeList users;
};

typedef RestrictionClassAssignment* RestrictionClassList; // size 1..254

class PositionalAccuracy
{
public:
    SemiMajorAxisAccuracy semiMajor;
    SemiMinorAxisAccuracy semiMinor;
    SemiMajorAxisOrientation orientation;
};

class AccelerationSet4Way
{
public:
    int16_t Long;
    int16_t lat;
    int8_t vert;
    int16_t yaw;
};

class BrakeAppliedStatus
{
public:

};

class BrakeSystemStatus
{
public:
    uint8_t wheelBrakes;
    TractionControlStatus traction;
    AntiLockBrakeStatus abs;
    StabilityControlStatus scs;
    BrakeBoostApplied brakeBoost;
    AuxiliaryBrakeStatus auxBrakes;
};

class VehicleSize
{
public:
    VehicleWidth width;
    VehicleLength length;
};

class BSMcoreData
{
public:
    MsgCount msgCnt;
    TemporaryID id;
    DSecond secMark;
    Latitude lat;
    Longitude Long;
    Elevation elev;
    PositionalAccuracy accuracy;
    TransmissionState transmission;
    Speed speed;
    Heading heading;
    SteeringWheelAngle angle;
    AccelerationSet4Way accelSet;
    BrakeSystemStatus brakes;
    VehicleSize size;
};

class BSMpartIIExtension
{
public:
};

class Reg_BasicSafetyMessage
{
public:
};

class TimeChangeDetails
{
public:
    TimeMark startTime;
    TimeMark minEndTime;
    TimeMark maxEndTime;
    TimeMark likelyTime;
    TimeIntervalConfidence confidence;
    TimeMark nextTime;
};

class AdvisorySpeed
{
public:
    AdvisorySpeedType type;
    SpeedAdvice speed;
    SpeedConfidence confidence;
    ZoneLength distance;
    RestrictionClassID Class;
};

typedef AdvisorySpeed* AdvisorySpeedList;

class MovementEvent
{
public:
    MovementPhaseState eventState;
    TimeChangeDetails timing;
    AdvisorySpeedList speeds;
};

class ConnectionManeuverAssist
{
public:
    LaneConnectionID connectionID;
    ZoneLength queueLength;
    ZoneLength availableStorageLength;
    WaitOnStopline waitOnStop;
    PedestrianBicycleDetect pedBicycleDetect;
};

typedef ConnectionManeuverAssist* ManeuverAssistList;


typedef MovementEvent* MovementEventList;

class MovementState
{
public:
    DescriptiveName movementName;
    SignalGroupID signalGroup;
    MovementEventList state_time_speed;
    ManeuverAssistList maneuverAssistList;
};

typedef MovementState* MovementList;

typedef LaneID* EnabledLaneList;

class IntersectionState
{
public:
    DescriptiveName name;
    IntersectionReferenceID id;
    MsgCount revision;
    IntersectionStatusObject status;
    MinuteOfTheYear moy;
    DSecond timeStamp;
    EnabledLaneList enabledLanes;
    MovementList states;
    ManeuverAssistList maneuverAssistList;
    RoadAuthorityID roadAuthorityID;
};

typedef IntersectionState* IntersectionStateList;

enum class VehicleID_type{
    entityID,
    stationID
};

union VehicleID_u{
    TemporaryID entityID;
    StationID stationID;
};

struct VehicleID{
    VehicleID_type type;
    VehicleID_u val;
};

enum class vehicleClass_type{
    vGroup,
    rGroup,
    rEquip
};

union vehicleClass_u{
    VehicleGroupAffected vGroup;
    ResponderGroupAffected rGroup;
    IncidentResponseEquipment rEquip;
};

struct VehicleClass{
    vehicleClass_type type;
    vehicleClass_u val;
};

class VehicleIdent{
public:
    DescriptiveName name;
    VINstring vin;
    char ownerCode[32];
    VehicleID id;
    VehicleType vehicleType;
    VehicleClass vehicleClass;
};

class DDateTime
{
public:
    DYear year;
    DMonth month;
    DDay day;
    DHour hour;
    DMinute minute;
    DSecond second;
    DOffset offset;
};

class TransmissionAndSpeed
{
public:
    TransmissionState transmission;
    Velocity speed;
};


class PositionConfidenceSet
{
public:
    PositionConfidence pos;
    ElevationConfidence elevation;
};

class SpeedandHeadingandThrottleConfidence
{
public:
    HeadingConfidence heading;
    SpeedConfidence speed;
    ThrottleConfidence throttle;
};

class FullPositionVector{
public:
    DDateTime utcTime ; //opt
    Longitude Long ;
    Latitude lat ;
    Elevation elev ; //opt
    Heading heading ; //opt
    TransmissionAndSpeed speed ; //opt
    PositionalAccuracy posAccuracy ; //opt
    TimeConfidence timeConfidence ; //opt
    PositionConfidenceSet posConfidence ; //opt
    SpeedandHeadingandThrottleConfidence speedConfidence ; //opt
};

class VehicleClassification
{
public:
    BasicVehicleClass keyType; //opt
    BasicVehicleRole role; //opt
    Iso3833VehicleType iso3883; //opt
    VehicleType hpmsType; //opt
    VehicleGroupAffected vehicleType; //opt
    IncidentResponseEquipment responseEquip; //opt
    ResponderGroupAffected responderType; //opt
    FuelType fuelType; //opt
};

class PathHistoryPoint
{
public:
    OffsetLL_B18 latOffset;
    OffsetLL_B18 lonOffset;
    VertOffset_B12 elevationOffset;
    TimeOffset timeOffset;
    Speed speed;
    PositionalAccuracy posAccuracy;
    CoarseHeading heading;
};

typedef PathHistoryPoint* PathHistoryPointList;

class PathHistory
{
public:
    FullPositionVector initialPosition;
    GNSSstatus currGNSSstatus;
    PathHistoryPointList crumbData;
};

class PathPrediction
{
public:
    RadiusOfCurvature radiusOfCurve;
    Confidence confidence;
};

class VehicleSafetyExtensions
{
public:
    VehicleEventFlags events;
    PathHistory pathHistory;
    PathPrediction pathPrediction;
    ExteriorLights lights;
    DDateTime timestamp;
    VehicleHeight height;
};

class WiperSet
{
public:
    WiperStatus statusFront;
    WiperRate rateFront;
    WiperStatus statusRear;
    WiperRate rateRear;
};

class AccelSteerYawRateConfidence
{
public:
    YawRateConfidence yawRate;
    AccelerationConfidence acceleration;
    SteeringWheelAngleConfidence steeringWheelAngle;
};

class ConfidenceSet
{
public:
    AccelSteerYawRateConfidence accelConfidence;
    SpeedandHeadingandThrottleConfidence speedConfidence;
    TimeConfidence timeConfidence;
    PositionConfidenceSet posConfidence;
    SteeringWheelAngleConfidence steerConfidence;
    HeadingConfidence headingConfidence;
    ThrottleConfidence throttleConfidence;
};

class BumperHeights
{
public:
    BumperHeight front;
    BumperHeight rear;
};

class TireData
{
public:
    TireLocation location;
    TirePressure pressure;
    TireTemp temp;
    WheelSensorStatus wheelSensorStatus;
    WheelEndElectFault wheelEndElectFault;
    TireLeakageRate leakageRate;
    TirePressureThresholdDetection detection;
};

typedef TireData* TireDataList;

class AxleWeightSet
{
public:
    AxleLocation location;
    AxleWeight weight;
};

typedef AxleWeightSet* AxleWeightList;

class J1939data
{
public:
    TireDataList tires;
    AxleWeightList axles;
    TrailerWeight trailerWeight;
    CargoWeight cargoWeight;
    SteeringAxleTemperature steeringAxleTemperature;
    DriveAxleLocation driveAxleLocation;
    DriveAxleLiftAirPressure driveAxleLiftAirPressure;
    DriveAxleTemperature driveAxleTemperature;
    DriveAxleLubePressure driveAxleLubePressure;
    SteeringAxleLubePressure steeringAxleLubePressure;
};

class VehicleStatus
{
public:
    ExteriorLights lights;
    LightbarInUse lightBar;
    WiperSet wipers;
    BrakeSystemStatus brakeStatus;
    BrakeAppliedPressure brakePressure;
    CoefficientOfFriction roadFriction;
    SunSensor sunData;
    RainSensor rainData;
    AmbientAirTemperature airTemp;
    AmbientAirPressure airPres;
    struct steering
    {
        SteeringWheelAngle angle;
        SteeringWheelAngleConfidence confidence;
        SteeringWheelAngleRateOfChange rate;
        DrivingWheelAngle wheels;
    };
    struct accelSets
    {
        AccelerationSet4Way accel4way;
        VerticalAccelerationThreshold vertAccelThres;
        YawRateConfidence yawRateCon;
        AccelerationConfidence hozAccelCon;
        ConfidenceSet confidenceSet;
    };
    struct Object{
        ObstacleDistance obDist;
        Angle obDirect;
        DDateTime dateTime;
    };
    FullPositionVector fullPos;
    ThrottlePosition throttlePos;
    SpeedandHeadingandThrottleConfidence speedHeadC;
    SpeedConfidence speedC;
    struct vehicleData
    {
        VehicleHeight height;
        BumperHeights bumpers;
        VehicleMass mass;
        TrailerWeight trailerWeight;
        VehicleType type;
    };
    VehicleIdent vehicleIdent;
    J1939data j1939data;
    struct weatherReport
    {
        EssPrecipYesNo isRaining;
        EssPrecipRate rainRate;
        EssPrecipSituation precipSituation;
        EssSolarRadiation solarRadiation;
        EssMobileFriction friction;
    };
    GNSSstatus gnssStatus;
};

class Snapshot
{
public:
    FullPositionVector thePosition;
    VehicleSafetyExtensions safetyExt;
    VehicleStatus dataSet;
};

class SnapshotList{
public:
    long size;
    Snapshot* val;
};