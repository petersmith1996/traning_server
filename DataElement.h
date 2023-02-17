//////////////////////////
//Data Element
//////////////////////////

typedef uint8_t SemiMajorAxisAccuracy;
typedef uint8_t SemiMinorAxisAccuracy;
typedef uint16_t SemiMajorAxisOrientation;

typedef bool WaitOnStopline;

typedef bool PedestrianBicycleDetect;

typedef uint16_t ZoneLength; //0..10000

enum class SpeedConfidence
{
    unavailable = 0,
    prec100ms,
    prec10ms,
    prec5ms,
    prec1ms,
    prec0_1ms,
    prec0_05ms,
    prec0_01ms,
};

typedef uint16_t SpeedAdvice; // 0..500, 0.1m/s^2

enum class AdvisorySpeedType
{
    none = 0,
    greenwave,
    ecoDrive,
    transit
};

typedef uint16_t TimeMark; // 0..36111

typedef uint8_t TimeIntervalConfidence; //0..15

enum class MovementPhaseState
{
    unavailable = 0,
    dark = 1,
    stop_Then_Proceed = 2,
    stop_And_Remain = 3,
    pre_Movement,
    permissive_Movement_Allowed,
    protected_Movement_Allowed,
    permissive_clearance,
    protected_clearance,
    caution_Conflicting_Traffic
};

enum class TractionControlStatus
{
    unavailable = 0,    // B’00 Not Equipped with traction control or traction control status is unavailable
    off = 1,            // B’01 traction control is Off
    on = 2,             // B’10 traction control is On (but not Engaged)
    engaged = 3         // B’11 traction control is Engaged
};

enum class AntiLockBrakeStatus
{
    unavailable = 0,    // B’00 Vehicle Not Equipped with ABS Brakes or ABS Brakes status is unavailable
    off = 1,            // B’01 Vehicle’s ABS are Off
    on = 2,             // B’10 Vehicle’s ABS are On ( but not Engaged )
    engaged = 3         // B’Vehicle’s ABS control is Engaged on any wheel
};

enum class StabilityControlStatus
{
    unavailable = 0,    // B’00 Not Equipped with SC or SC status is unavailable
    off = 1,            // B’01 Off
    on = 2,             // B’10 On or active (but not engaged)
    engaged = 3         // B’stability control is Engaged
};

enum class BrakeBoostApplied
{
    unavailable = 0,    // B’00 Vehicle not equipped with brake boost or brake boost data is unavailable
    off = 1,            // B’01 Vehicle’s brake boost is off
    on = 2,             // B’10 Vehicle’s brake boost is on (applied)
};

enum class AuxiliaryBrakeStatus
{
    unavailable = 0,    // B’00 Vehicle Not Equipped with Aux Brakes or Aux Brakes status is unavailable
    off = 1,            // B’01 Vehicle’s Aux Brakes are Off
    on = 2,             // B’10 Vehicle’s Aux Brakes are On ( Engaged )
    reserved = 3        // B’11
};

enum class TransmissionState
{
    neutral = 0,         // Neutral
    park = 1,            // Park
    forwardGears = 2,    // Forward gears
    reverseGears = 3,    // Reverse gears
    reserved1 = 4,
    reserved2 = 5,
    reserved3 = 6,
    unavailable = 7      // not-equipped or unavailable value, Any related speed is relative to the vehicle reference frame used
};

typedef uint32_t MinuteOfTheYear; // 0..527040

typedef uint8_t MsgCount; // 0..127

typedef char TemporaryID[4];

typedef uint16_t DSecond; // 0..65535

enum class LayerType
{
    none,
    mixedContent,
    generalMapData,
    intersectionData,
    curveData,
    roadwaySectionData,
    parkingAreaData,
    sharedLaneData
};

typedef uint8_t LayerID; // 0..100

typedef char DescriptiveName[63]; // size 1..63

typedef uint16_t RoadRegulatorID; // 0..65535

typedef uint16_t IntersectionID; // 0..65535

typedef int32_t Latitude; // -90000000..900000001

typedef int32_t Longitude; // -1799999999..1800000001

typedef int32_t Elevation; // -4096..61439

typedef uint16_t LaneWidth; // 0..32767

typedef uint16_t Velocity; //0..8191

typedef uint16_t Speed; //0..8191

enum class SpeedLimitType
{
    unknown, 
    maxSpeedInSchoolZone, 
    maxSpeedInSchoolZoneWhenChildrenArePresent,
    maxSpeedInConstructionZone, 
    vehicleMinSpeed,
    vehicleMaxSpeed,
    vehicleNightMaxSpeed,
    truckMinSpeed,
    truckMaxSpeed,
    truckNightMaxSpeed,
    vehiclesWithTrailersMinSpeed,
    vehiclesWithTrailersMaxSpeed,
    vehiclesWithTrailersNightMaxSpeed
};

typedef uint8_t LaneID; // 0..255

typedef uint8_t ApproachID; // 0..15

typedef uint8_t LaneDirection; // bit string, size 2
typedef uint16_t LaneSharing; //bit string size 10
typedef uint8_t LaneAttributes_Vehicle; // bit string, size 8
typedef uint16_t LaneAttributes_Crosswalk; // bit string, size 16
typedef uint16_t LaneAttributes_Bike; // bit string, size 16
typedef uint16_t LaneAttributes_Sidewalk; // bit string, size 16
typedef uint16_t LaneAttributes_Barrier; // bit string, size 16
typedef uint16_t LaneAttributes_Striping; // bit string, size 16
typedef uint16_t LaneAttributes_TrackedVehicle; // bit string, size 16
typedef uint16_t LaneAttributes_Parking; // bit string, size 16
typedef uint16_t AllowedManeuvers; //bit string, size 12

typedef int16_t Offset_B10; // -512..511
typedef int16_t Offset_B11; // -1024..1023
typedef int16_t Offset_B12; // -2048..2047
typedef int16_t Offset_B13; // -4096..4095
typedef int16_t Offset_B14; // -8192..8191
typedef int16_t Offset_B16; // -32768..32767

typedef int16_t DrivenLineOffsetSm; // -2047..2047
typedef int16_t DrivenLineOffsetLg; // -32767..32767

typedef uint16_t Angle; // 0..28800

typedef int16_t Scale_B12; // -2048..2047

enum class NodeAttributeXY{
    reserved,
    stopLine,
    roundedCapStyleA,
    roundedCapStyleB,
    mergePoint,
    divergePoint,
    downstreamStopLine,
    downstreamStartNode,
    closedToTraffic,
    safeIsland,
    curbPresentAtStepOff,
    hydrantPresent
};

enum class SegmentAttributeXY{
    reserved,
    doNotBlock,
    whiteLine,
    mergingLaneLeft,
    mergingLaneRight,
    curbOnLeft ,
    curbOnRight,
    loadingzoneOnLeft,
    loadingzoneOnRight,
    turnOutPointOnLeft,
    turnOutPointOnRight,
    adjacentParkingOnLeft,
    adjacentParkingOnRight,
    adjacentBikeLaneOnLeft,
    adjacentBikeLaneOnRight,
    sharedBikeLane,
    bikeBoxInFront,
    transitStopOnLeft,
    transitStopOnRight,
    transitStopInLane,
    sharedWithTrackedVehicle,
    safeIsland,
    lowCurbsPresent,
    rumbleStripPresent,
    audibleSignalingPresent,
    adaptiveTimingPresent,
    rfSignalRequestPresent,
    partialCurbIntrusion,
    taperToLeft,
    taperToRight,
    taperToCenterLine,
    parallelParking,
    headInParking,
    freeParking,
    timeRestrictionsOnParking,
    costToPark,
    midBlockCurbPresent,
    unEvenPavementPresent
};

typedef int16_t DeltaAngle; // -150..150
typedef int8_t RoadwayCrownAngle; // -128..127
typedef int16_t MergeDivergeNodeAngle; // -180..180
typedef uint8_t SignalGroupID; // 0..255
typedef uint8_t RestrictionClassID; // 0..255
typedef uint8_t LaneConnectionID; // 0..255

typedef uint16_t RoadSegmentID; // 0..65535

typedef uint8_t RoadLaneSetList; // 0..255

enum class RestrictionAppliesTo{
    none, 
    equippedTransit, 
    equippedTaxis,
    equippedOther, 
    emissionCompliant, 
    equippedBicycle,
    weightCompliant,
    heightCompliant,
    pedestrians,
    slowMovingPersons,
    wheelchairUsers,
    visualDisabilities,
    audioDisabilities, 
};

typedef uint16_t Heading;

typedef int8_t SteeringWheelAngle;

typedef uint16_t VehicleWidth;
typedef uint16_t VehicleLength;

typedef uint16_t IntersectionStatusObject; // bit string, size 16

typedef uint16_t ProbeSegmentNumber;

typedef char VINstring[17];

typedef uint32_t StationID;

enum class VehicleType{
    none = 0,
    unknown = 1,
    special = 2,
    moto = 3,
    car = 4,
    carOther = 5,
    bus = 6,
    axleCnt2 = 7,
    axleCnt3 = 8,
    axleCnt4 = 9,
    axleCnt4Trailer = 10,
    axleCnt5Trailer = 11,
    axleCnt6Trailer = 12,
    axleCnt5MultiTrailer = 13,
    axleCnt6MultiTrailer = 14,
    axleCnt7MultiTrailer = 15,
};


enum class VehicleGroupAffected
{
    all_vehicles = 9217,
    bicycles = 9218,
    motorcycles = 9219,
    cars = 9220,
    light_vehicles = 9221,
    cars_and_light_vehicles = 9222,
    cars_with_trailers = 9223,
    cars_with_recreational_trailers = 9224,
    vehicles_with_trailers = 9225,
    heavy_vehicles = 9226,
    trucks = 9227,
    buses = 9228,
    articulated_buses = 9229,
    school_buses = 9230,
    vehicles_with_semi_trailers = 9231,
    vehicles_with_double_trailers = 9232,
    high_profile_vehicles = 9233,
    wide_vehicles = 9234,
    long_vehicles = 9235,
    hazardous_loads = 9236,
    exceptional_loads = 9237,
    abnormal_loads = 9238,
    convoys = 9239,
    maintenance_vehicles = 9240,
    delivery_vehicles = 9241,
    vehicles_with_even_numbered_license_plates = 9242,
    vehicles_with_odd_numbered_license_plates = 9243,
    vehicles_with_parking_permits = 9244,
    vehicles_with_catalytic_converters = 9245,
    vehicles_without_catalytic_converters = 9246,
    gas_powered_vehicles = 9247,
    diesel_powered_vehicles = 9248,
    lPG_vehicles = 9249,
    military_convoys = 9250,
    military_vehicles = 9251
};

enum class ResponderGroupAffected
{
    emergency_vehicle_units = 9729,
    federal_law_enforcement_units = 9730,
    state_police_units = 9731,
    county_police_units = 9732,
    local_police_units = 9733,
    ambulance_units = 9734,
    rescue_units = 9735,
    fire_units = 9736,
    hAZMAT_units = 9737,
    light_tow_unit = 9738,
    heavy_tow_unit = 9739,
    freeway_service_patrols = 9740,
    transportation_response_units = 9741,
    private_contractor_response_units = 9742
};

enum class IncidentResponseEquipment
{
    ground_fire_suppression = 9985,
    heavy_ground_equipment = 9986,
    aircraft = 9988,
    marine_equipment = 9989,
    support_equipment = 9990,
    medical_rescue_unit = 9991,
    other = 9993,
    ground_fire_suppression_other = 9994,
    engine = 9995,
    truck_or_aerial = 9996,
    quint = 9997,
    tanker_pumper_combination = 9998,
    brush_truck = 10000,
    aircraft_rescue_firefighting = 10001,
    heavy_ground_equipment_other = 10004,
    dozer_or_plow = 10005,
    tractor = 10006,
    tanker_or_tender = 10008,
    aircraft_other = 10024,
    aircraft_fixed_wing_tanker = 10025,
    helitanker = 10026,
    helicopter = 10027,
    marine_equipment_other = 10034,
    fire_boat_with_pump = 10035,
    boat_no_pump = 10036,
    support_apparatus_other = 10044,
    breathing_apparatus_support = 10045,
    light_and_air_unit = 10046,
    medical_rescue_unit_other = 10054,
    rescue_unit = 10055,
    urban_search_rescue_unit = 10056,
    high_angle_rescue = 10057,
    crash_fire_rescue = 10058,
    bLS_unit = 10059,
    aLS_unit = 10060,
    mobile_command_post = 10075,
    chief_officer_car = 10076,
    hAZMAT_unit = 10077,
    type_i_hand_crew = 10078,
    type_ii_hand_crew = 10079,
    privately_owned_vehicle = 10083,
    other_apparatus_resource = 10084,
    ambulance = 10085,
    bomb_squad_van = 10086,
    combine_harvester = 10087,
    construction_vehicle = 10088,
    farm_tractor = 10089,
    grass_cutting_machines = 10090,
    hAZMAT_containment_tow = 10091,
    heavy_tow = 10092,
    light_tow = 10094,
    flatbed_tow = 10114,
    hedge_cutting_machines = 10093,
    mobile_crane = 10095,
    refuse_collection_vehicle = 10096,
    resurfacing_vehicle = 10097,
    road_sweeper = 10098,
    roadside_litter_collection_crews = 10099,
    salvage_vehicle = 10100,
    sand_truck = 10101,
    snowplow = 10102,
    steam_roller = 10103,
    swat_team_van = 10104,
    track_laying_vehicle = 10105,
    unknown_vehicle = 10106,
    white_lining_vehicle = 10107,
    dump_truck = 10108,
    supervisor_vehicle = 10109,
    snow_blower = 10110,
    rotary_snow_blower = 10111,
    road_grader = 10112,
    steam_truck = 10113
};

typedef struct RelativeRoadAuthorityID{
} RelativeRoadAuthorityID;

typedef struct FullRoadAuthorityID{
    unsigned short  length;
    unsigned char   *value;
} FullRoadAuthorityID;

typedef uint8_t DYear;
typedef uint8_t DMonth;
typedef uint8_t DDay;
typedef uint8_t DHour;
typedef uint8_t DMinute;
typedef uint16_t DOffset;

enum class TimeConfidence
{
    unavailable = 0,
    time_100_000 = 1,
    time_050_000 = 2,
    time_020_000 = 3,
    time_010_000 = 4,
    time_002_000 = 5,
    time_001_000 = 6,
    time_000_500 = 7,
    time_000_200 = 8,
    time_000_100 = 9,
    time_000_050 = 10,
    time_000_020 = 11,
    time_000_010 = 12,
    time_000_005 = 13,
    time_000_002 = 14,
    time_000_001 = 15,
    time_000_000_5 = 16,
    time_000_000_2 = 17,
    time_000_000_1 = 18,
    time_000_000_05 = 19,
    time_000_000_02 = 20,
    time_000_000_01 = 21,
    time_000_000_005 = 22,
    time_000_000_002 = 23,
    time_000_000_001 = 24,
    time_000_000_000_5 = 25,
    time_000_000_000_2 = 26,
    time_000_000_000_1 = 27,
    time_000_000_000_05 = 28,
    time_000_000_000_02 = 29,
    time_000_000_000_01 = 30,
    time_000_000_000_005 = 31,
    time_000_000_000_002 = 32,
    time_000_000_000_001 = 33,
    time_000_000_000_000_5 = 34,
    time_000_000_000_000_2 = 35,
    time_000_000_000_000_1 = 36,
    time_000_000_000_000_05 = 37,
    time_000_000_000_000_02 = 38,
    time_000_000_000_000_01 = 39
};

enum class PositionConfidence{
    unavailable = 0,
    a500m = 1,
    a200m = 2,
    a100m = 3,
    a50m = 4,
    a20m = 5,
    a10m = 6,
    a5m = 7,
    a2m = 8,
    a1m = 9,
    a50cm = 10,
    a20cm = 11,
    a10cm = 12,
    a5cm = 13,
    a2cm = 14,
    a1cm = 15
};

enum class HeadingConfidence
{
    unavailable = 0,
    prec10deg = 1,
    prec05deg = 2,
    prec01deg = 3,
    prec0_1deg = 4,
    prec0_05deg = 5,
    prec0_01deg = 6,
    prec0_0125deg = 7
};

enum class ThrottleConfidence
{
    unavailable,
    prec10percent,
    prec1percent,
    prec0_5percent
};

enum class ElevationConfidence
{
    unavailable = 0,
    elev_500_00 = 1,
    elev_200_00 = 2,
    elev_100_00 = 3,
    elev_050_00 = 4,
    elev_020_00 = 5,
    elev_010_00 = 6,
    elev_005_00 = 7,
    elev_002_00 = 8,
    elev_001_00 = 9,
    elev_000_50 = 10,
    elev_000_20 = 11,
    elev_000_10 = 12,
    elev_000_05 = 13,
    elev_000_02 = 14,
    elev_000_01 = 15
};

typedef uint8_t BasicVehicleClass;

enum class BasicVehicleRole
{
    basicVehicle = 0,
    publicTransport = 1,
    specialTransport = 2,
    dangerousGoods = 3,
    roadWork = 4,
    roadRescue = 5,
    emergency = 6,
    safetyCar = 7,
    none_unknown = 8,
    truck = 9,
    motorcycle = 10,
    roadSideSource = 11,
    police = 12,
    fire = 13,
    ambulance = 14,
    dot = 15,
    transit = 16,
    slowMoving = 17,
    stopNgo = 18,
    cyclist = 19,
    pedestrian = 20,
    nonMotorized = 21,
    military = 22
};

typedef uint8_t Iso3833VehicleType;

typedef uint8_t FuelType;

typedef uint16_t VehicleEventFlags;

typedef uint8_t GNSSstatus;

typedef uint16_t OffsetLL_B18;

typedef uint16_t VertOffset_B12;

typedef uint16_t TimeOffset;

typedef uint8_t CoarseHeading;

typedef uint16_t RadiusOfCurvature;

typedef uint8_t Confidence;

typedef uint16_t ExteriorLights;

typedef uint8_t VehicleHeight;

enum class LightbarInUse
{
    unavailable = 0,
    notInUse = 1,
    inUse = 2,
    yellowCautionLights = 3,
    schooldBusLights = 4,
    arrowSignsActive = 5,
    slowMovingVehicle = 6,
    freqStops = 7
};

enum class BrakeAppliedPressure
{
    unavailable = 0,
    minPressure = 1,
    bkLvl_2 = 2,
    bkLvl_3 = 3,
    bkLvl_4 = 4,
    bkLvl_5 = 5,
    bkLvl_6 = 6,
    bkLvl_7 = 7,
    bkLvl_8 = 8,
    bkLvl_9 = 9,
    bkLvl_10 = 10,
    bkLvl_11 = 11,
    bkLvl_12 = 12,
    bkLvl_13 = 13,
    bkLvl_14 = 14,
    maxPressure = 15
};

typedef uint8_t CoefficientOfFriction;

typedef uint16_t SunSensor;

enum class RainSensor
{
    none = 0,
    lightMist = 1,
    heavyMist = 2,
    lightRainOrDrizzle = 3,
    rain = 4,
    moderateRain = 5,
    heavyRain = 6,
    heavyDownpour = 7
};

typedef uint8_t AmbientAirTemperature;

typedef uint8_t AmbientAirPressure;

enum class SteeringWheelAngleConfidence
{
    unavailable = 0,
    prec2deg = 1,
    prec1deg = 2,
    prec0_02deg = 3
};

typedef int8_t SteeringWheelAngleRateOfChange;

typedef int8_t DrivingWheelAngle;

typedef uint8_t VerticalAccelerationThreshold;

enum class YawRateConfidence
{
    unavailable = 0,
    degSec_100_00 = 1,
    degSec_010_00 = 2,
    degSec_005_00 = 3,
    degSec_001_00 = 4,
    degSec_000_10 = 5,
    degSec_000_05 = 6,
    degSec_000_01 = 7
};

enum class AccelerationConfidence
{
    unavailable = 0,
    accl_100_00 = 1,
    accl_010_00 = 2,
    accl_005_00 = 3,
    accl_001_00 = 4,
    accl_000_10 = 5,
    accl_000_05 = 6,
    accl_000_01 = 7
};

typedef uint8_t ObstacleDistance;

typedef uint16_t ThrottlePosition;

typedef uint8_t VehicleMass;
typedef uint16_t TrailerWeight;
typedef uint8_t TireLocation;
typedef uint8_t TirePressure;
typedef int32_t TireTemp;

enum class WheelSensorStatus
{
    off = 0,
    on = 1,
    notDefined = 2,
    notSupported = 3
};

enum class WheelEndElectFault
{
    isOk = 0,
    isNotDefined = 1,
    isError = 2,
    isNotSupported = 3
};

typedef uint16_t TireLeakageRate;

enum class TirePressureThresholdDetection
{
    noData = 0,
    overPressure = 1,
    noWarningPressure = 2,
    underPressure = 3,
    extremeUnderPressure = 4,
    undefined = 5,
    errorIndicator = 6,
    notAvailable = 7
};

typedef uint8_t AxleLocation;
typedef uint16_t AxleWeight;
typedef uint16_t CargoWeight;
typedef int16_t SteeringAxleTemperature;
typedef uint8_t DriveAxleLocation;
typedef uint16_t DriveAxleLiftAirPressure;
typedef int16_t DriveAxleTemperature;
typedef uint8_t DriveAxleLubePressure;
typedef uint8_t SteeringAxleLubePressure;


enum class EssPrecipYesNo
{
    precip = 1,
    noPrecip = 2,
    error = 3
};

enum class EssPrecipSituation
{
    other = 1,
    unknown = 2,
    noPrecipitation = 3,
    unidentifiedSlight = 4,
    unidentifiedModerate = 5,
    unidentifiedHeavy = 6,
    snowSlight = 7,
    snowModerate = 8,
    snowHeavy = 9,
    rainSlight = 10,
    rainModerate = 11,
    rainHeavy = 12,
    frozenPrecipitationSlight = 13,
    frozenPrecipitationModerate = 14,
    frozenPrecipitationHeavy = 15
};

typedef uint16_t EssPrecipRate;
typedef uint16_t EssSolarRadiation;
typedef uint8_t EssMobileFriction;

enum class WiperStatus
{
    unavailable = 0,
    off = 1,
    intermittent = 2,
    low = 3,
    high = 4,
    washerInUse = 5,
    automaticPresent = 6,
};

typedef uint8_t WiperRate;
typedef uint8_t BumperHeight;