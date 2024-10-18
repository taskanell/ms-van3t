/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 * Created by:
 *  Carlos Mateo Risma Carletti, Politecnico di Torino (carlosrisma@gmail.com)
*/

#include "cpBasicService.h"
#include "ns3/snr-tag.h"
#include "ns3/sinr-tag.h"
#include "ns3/rssi-tag.h"
#include "ns3/timestamp-tag.h"
#include "ns3/rsrp-tag.h"
#include "ns3/size-tag.h"

//needed -struct to json- libraries

//#include "struct_mapping/struct_mapping.h"
#include "nlohmann-json/single_include/nlohmann/json.hpp"

using json = nlohmann::ordered_json;

#include <iostream>
#include <optional>
#include <sstream>
#include <string>

void empty_json_file(const std::string& filename) {
    std::ofstream file(filename, std::ofstream::trunc);  // Open the file in truncation mode
    if (file.is_open()) 
    {
        file << "{}";
        file.close();  // Close the file after truncating (file will be empty)
    } 
    else 
    {
        std::cerr << "Could not open the file for clearing: " << filename << std::endl;
    }
}


void to_json(json&j,const CollectivePerceptionMessage &_cpm, const std::string filename, long gendeltatime)
{
  
  json existing_data;
  std::ifstream file_in(filename);
  if (file_in.is_open()) 
  {
      try 
      {
          // Parse the existing data
          file_in >> existing_data;
      } 
      catch (const std::exception& e) 
      {
      std::cerr << "Error reading JSON from file: " << e.what() << std::endl;
      }
      file_in.close();  // Close the input stream
  }

  j ["header"]["messageId"] = _cpm.header.messageId;
  j ["header"]["protocolVersion"]= _cpm.header.protocolVersion;
  j["header"]["stationId"]= _cpm.header.stationId;

  j["payload"]["managementContainer"]["referenceTime"]=gendeltatime;  //pass it manually (needs conversion for timestamp_t to integer) 
  j["payload"]["managementContainer"]["referencePosition"]["latitude"]\
  =_cpm.payload.managementContainer.referencePosition.latitude;
  j["payload"]["managementContainer"]["referencePosition"]["longitude"]\
  =_cpm.payload.managementContainer.referencePosition.longitude;
  j["payload"]["managementContainer"]["referencePosition"]["altitude"]["altitudeValue"]\
  =_cpm.payload.managementContainer.referencePosition.altitude.altitudeValue;
  
  for(int i=0; i<_cpm.payload.cpmContainers.list.count; i++) 
  {
    std::string cpmContainer = "cpmContainer" + std::to_string(i);

    j["payload"]["cpmContainers"][cpmContainer]["containerId"]\
    =_cpm.payload.cpmContainers.list.array[i]->containerId;
    
    switch(_cpm.payload.cpmContainers.list.array[i]->containerId)
    {
      case 1:
        j["payload"]["cpmContainers"][cpmContainer]["OriginatingVehicleContainer"]["orientationAngle"]["value"]\
        =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.OriginatingVehicleContainer.orientationAngle.value;
        j["payload"]["cpmContainers"][cpmContainer]["OriginatingVehicleContainer"]["orientationAngle"]["confidence"]\
        =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.OriginatingVehicleContainer.orientationAngle.confidence;
        break;
      case 3:
        for(int z=0; z<_cpm.payload.cpmContainers.list.array[i]->containerData.choice.SensorInformationContainer.list.count; z++)
        {
          std::string sensorInfo = "SensorInformation" + std::to_string(z);

          j["payload"]["cpmContainers"][cpmContainer]["SensorInformationContainer"]["SensorInformation"][sensorInfo]["sensorId"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.SensorInformationContainer.list.array[z]->sensorId;
          j["payload"]["cpmContainers"][cpmContainer]["SensorInformationContainer"]["SensorInformation"][sensorInfo]["sensorType"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.SensorInformationContainer.list.array[z]->sensorType;
          j["payload"]["cpmContainers"][cpmContainer]["SensorInformationContainer"]["SensorInformation"][sensorInfo]["shadowAplies"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.SensorInformationContainer.list.array[z]->shadowingApplies;

          j["payload"]["cpmContainers"][cpmContainer]["SensorInformationContainer"]["SensorInformation"][sensorInfo]["perceptionRegionShape"]\
          ["type"]=_cpm.payload.cpmContainers.list.array[i]->containerData.choice.SensorInformationContainer.list.array[z]->\
          perceptionRegionShape->present;
          j["payload"]["cpmContainers"][cpmContainer]["SensorInformationContainer"]["SensorInformation"][sensorInfo]["perceptionRegionShape"]\
          ["radius"]=_cpm.payload.cpmContainers.list.array[i]->containerData.choice.SensorInformationContainer.list.array[z]->\
          perceptionRegionShape->choice.circular.radius;
          j["payload"]["cpmContainers"][cpmContainer]["SensorInformationContainer"]["SensorInformation"][sensorInfo]["perceptionRegionShape"]\
          ["referencePoint"]["xCoordinate"]=_cpm.payload.cpmContainers.list.array[i]->containerData.choice.SensorInformationContainer.list.array[z]->\
          perceptionRegionShape->choice.circular.shapeReferencePoint->xCoordinate;
          j["payload"]["cpmContainers"][cpmContainer]["SensorInformationContainer"]["SensorInformation"][sensorInfo]["perceptionRegionShape"]\
          ["referencePoint"]["yCoordinate"]=_cpm.payload.cpmContainers.list.array[i]->containerData.choice.SensorInformationContainer.list.array[z]->\
          perceptionRegionShape->choice.circular.shapeReferencePoint->yCoordinate;
        }
        break;
      case 5:
        for(int z=0; z<_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.count; z++)
        {
          std::string perceivedObj = "PerceivedObject" + std::to_string(z);

          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["numberofPerceivedObjects"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.numberOfPerceivedObjects;

          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["objectId"]\
          =*(_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->objectId);
          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["measurementDeltaTime"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->measurementDeltaTime;

          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["position"]["xCoordinate"]["value"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->position.xCoordinate.value;
          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["position"]["xCoordinate"]["confidence"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->position.xCoordinate.confidence;
          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["position"]["yCoordinate"]["value"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->position.yCoordinate.value;
          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["position"]["yCoordinate"]["confidence"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->position.yCoordinate.confidence;

          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["velocity"]["xVelocity"]["value"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->velocity->choice.cartesianVelocity\
          .xVelocity.value;
          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["velocity"]["xVelocity"]["confidence"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->velocity->choice.cartesianVelocity\
          .xVelocity.confidence;
          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["velocity"]["yVelocity"]["value"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->velocity->choice.cartesianVelocity\
          .yVelocity.value;
          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["velocity"]["yVelocity"]["confidence"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->velocity->choice.cartesianVelocity\
          .yVelocity.confidence;

          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["acceleration"]["xAcceleration"]["value"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->acceleration->choice.cartesianAcceleration\
          .xAcceleration.value;
          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["acceleration"]["xAcceleration"]["confidence"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->acceleration->choice.cartesianAcceleration\
          .xAcceleration.confidence;
          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["acceleration"]["yAcceleration"]["value"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->acceleration->choice.cartesianAcceleration\
          .yAcceleration.value;
          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["acceleration"]["yAcceleration"]["confidence"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->acceleration->choice.cartesianAcceleration\
          .yAcceleration.confidence;

          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["angles"]["zAngle"]["value"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->angles->zAngle.value;
          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["angles"]["zAngle"]["confidence"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->angles->zAngle.confidence;

          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["objectDimensionX"]["value"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->objectDimensionX->value;
          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["objectDimensionX"]["confidence"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->objectDimensionX->confidence;

          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["objectDimensionY"]["value"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->objectDimensionY->value;
          j["payload"]["cpmContainers"][cpmContainer]["PerceivedObjectContainer"]["PerceivedObjects"][perceivedObj]["objectDimensionY"]["confidence"]\
          =_cpm.payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[z]->objectDimensionY->confidence;
        }
        break;
        
    }

    if (existing_data.is_array()) {
        existing_data.push_back(j);  // Append to an existing JSON array
    } else if (existing_data.empty()) {
        // If the file was empty, initialize it as an array and add the new data
        existing_data = json::array();
        existing_data.push_back(j);
    } else {
        // If the file contains a JSON object, you can also convert it into an array
        json temp = json::array();
        temp.push_back(existing_data);  // Keep the previous object in the array
        temp.push_back(j);  // Add the new data
        existing_data = temp;  // Assign the array back to `j`
    }

    std::ofstream file_out(filename);
    if (file_out.is_open())
    {
      file_out << existing_data.dump(4);
      file_out.close();
    }
    else
    {
      std::cerr << "Could not open the file for writing: " << filename << std::endl;
    }
  } 
  
 
}

/*WrappedCpmContainer* add_new_wrcpm(struct WrappedCpmContainers &wrcpms, int containerid_val)
{
  WrappedCpmContainer* wrcpm = (WrappedCpmContainer*)malloc(sizeof(WrappedCpmContainer));
  //wrcpm2.containerData.choice.OriginatingVehicleContainer.orientationAngle.value = cpm_mandatory_data.heading.getValue ();
  wrcpm->containerId = containerid_val;
  //std::cout << "IDCONT: " << wrcpm2->containerId << std::endl;

  wrcpms.list.array[wrcpms.list.count] = wrcpm;
  wrcpms.list.array = (struct WrappedCpmContainer **)realloc(wrcpms.list.array, ++wrcpms.list.size * sizeof(struct WrappedCpmContainer *));
  wrcpms.list.count++;
  return wrcpm;
}*/

//case for only creation of list of containers
/*template<typename T,typename V>
void add_new_item_to_container(T &containers)
{
  V* new_container = (V*)malloc(sizeof(V));
  
  containers.list.array[containers.list.count] = new_container;
  containers.list.array = (V**)realloc(containers.list.array, ++containers.list.size * sizeof(V *));
  containers.list.count++;

}*/

//simple case for only container creation and appending to the list of containers
template<typename T,typename V>
V* add_new_item_to_container(T &containers)
{
  V* new_container = (V*)malloc(sizeof(V));

  //std::cout << "SID: " << new_container->perceptionRegionShape->choice.circular.radius <<std::endl;
  
  containers.list.array[containers.list.count] = new_container;
  containers.list.array = (V**)realloc(containers.list.array, ++containers.list.size * sizeof(V *));
  containers.list.count++;

  return new_container;

}

//case for PerceivedObjects container creation and filler
template<typename T,typename V>
void add_new_item_to_container(T &containers,\
Identifier2B_t *objectid,DeltaTimeMilliSecondSigned_t mestime,CartesianPosition3dWithConfidence_t pos,Velocity3dWithConfidence_t *vel,\
Acceleration3dWithConfidence_t *acc,EulerAnglesWithConfidence_t	*ang,ObjectDimension_t	*objDimensionX,\
ObjectDimension_t	*objDimensionY)
{

  V* new_container = (V*)malloc(sizeof(V));
  //PerceivedObject *p;
  //p->angles->zAngle.

  //objectid
  new_container->objectId = (Identifier2B_t*)malloc(sizeof(Identifier2B_t));
  *(new_container->objectId) = *objectid;

  //timestamp
  new_container->measurementDeltaTime = mestime;
  
  //position
  new_container->position.xCoordinate.value = pos.xCoordinate.value;
  new_container->position.xCoordinate.confidence = pos.xCoordinate.confidence;
  new_container->position.yCoordinate.value = pos.yCoordinate.value;
  new_container->position.yCoordinate.confidence = pos.yCoordinate.confidence;

  //velocity
  new_container->velocity = (Velocity3dWithConfidence_t*)malloc(sizeof(Velocity3dWithConfidence_t));
  new_container->velocity->choice.cartesianVelocity.xVelocity.value = vel->choice.cartesianVelocity.xVelocity.value;
  new_container->velocity->choice.cartesianVelocity.xVelocity.confidence = vel->choice.cartesianVelocity.xVelocity.confidence;
  new_container->velocity->choice.cartesianVelocity.yVelocity.value = vel->choice.cartesianVelocity.yVelocity.value;
  new_container->velocity->choice.cartesianVelocity.yVelocity.confidence = vel->choice.cartesianVelocity.yVelocity.confidence;

  //acceleration
  new_container->acceleration = (Acceleration3dWithConfidence_t*)malloc(sizeof(Acceleration3dWithConfidence_t));
  new_container->acceleration->choice.cartesianAcceleration.xAcceleration.value = acc->choice.cartesianAcceleration.xAcceleration.value;
  new_container->acceleration->choice.cartesianAcceleration.xAcceleration.confidence = acc->choice.cartesianAcceleration.xAcceleration.confidence;
  new_container->acceleration->choice.cartesianAcceleration.yAcceleration.value = acc->choice.cartesianAcceleration.yAcceleration.value;
  new_container->acceleration->choice.cartesianAcceleration.yAcceleration.confidence = acc->choice.cartesianAcceleration.yAcceleration.confidence;

  //angles
  new_container->angles = (EulerAnglesWithConfidence_t*)malloc(sizeof(EulerAnglesWithConfidence_t));
  new_container->angles->zAngle.value = ang->zAngle.value;
  new_container->angles->zAngle.confidence = ang->zAngle.confidence;

  //objectdimension
  new_container->objectDimensionX = (ObjectDimension_t*)malloc(sizeof(ObjectDimension_t));
  new_container->objectDimensionY = (ObjectDimension_t*)malloc(sizeof(ObjectDimension_t));
  new_container->objectDimensionX->value = objDimensionX->value;
  new_container->objectDimensionY->value = objDimensionY->value;
  new_container->objectDimensionX->confidence = objDimensionX->confidence;
  new_container->objectDimensionY->confidence = objDimensionY->confidence;
  //std::cout << "OBJECTLVALUE: " << *(new_container->objectId) << std::endl;
  
  containers.list.array[containers.list.count] = new_container;
  //std::cout << "HERE" << std::endl;
  containers.list.array = (V**)realloc(containers.list.array, ++containers.list.size * sizeof(V *));
  //std::cout << "HERE 2" << std::endl;
  containers.list.count++;

}

//case for containerId = 1
template<typename T,typename V>
void add_new_item_to_container(T &containers,long containerid_val, long value, long confidence)
{
  V* new_container = (V*)malloc(sizeof(V));

  new_container->containerId = containerid_val;
  new_container->containerData.choice.OriginatingVehicleContainer.orientationAngle.value = value;
  new_container->containerData.choice.OriginatingVehicleContainer.orientationAngle.value = confidence;
  
  containers.list.array[containers.list.count] = new_container;
  containers.list.array = (V**)realloc(containers.list.array, ++containers.list.size * sizeof(V *));
  containers.list.count++;

}

//case for containerId = 3 (only one sensor currently->needs adaptation for more sensors)
template<typename T,typename V>
void add_new_item_to_container(T &containers,long containerid_val, SensorInformationContainer &sencon,\
long sensor_id,long sensor_type,BOOLEAN_t sh_Applies,Shape_PR pres,long radius,long x_cor,long y_cor)
{
  
  SensorInformation* sen_info = add_new_item_to_container<SensorInformationContainer,SensorInformation>(sencon);
  //std::cout << "POINTER: " << sen_info->perceptionRegionShape->choice << std::endl;
  //std::cout << "SID: " << sen_info->perceptionRegionShape->choice.circular.radius <<std::endl;
  sen_info->sensorId = sensor_id;
  sen_info->sensorType = sensor_type;
  sen_info->shadowingApplies = sh_Applies;

  sen_info->perceptionRegionShape = (Shape*)malloc(sizeof(Shape));
  sen_info->perceptionRegionShape->present = pres;
  sen_info->perceptionRegionShape->choice.circular.radius = radius;
  //std::cout << "I AM HERE!!!" << std::endl;

  sen_info->perceptionRegionShape->choice.circular.shapeReferencePoint = (CartesianPosition3d*)malloc(sizeof(CartesianPosition3d));
  sen_info->perceptionRegionShape->choice.circular.shapeReferencePoint->xCoordinate = x_cor;
  sen_info->perceptionRegionShape->choice.circular.shapeReferencePoint->yCoordinate = y_cor;

  //std::cout << "I AM HERE" << std::endl;

  V* new_container = (V*)malloc(sizeof(V));

  //WrappedCpmContainer* wr;

  //wr->containerData.choice.SensorInformationContainer
 //std::cout << "I AM HERE2" << std::endl;
  new_container->containerId = containerid_val;
  new_container->containerData.choice.SensorInformationContainer= sencon;
  //WrappedCpmContainer* con;
  //std::cout << "I AM HERE3" << std::endl;
  
  containers.list.array[containers.list.count] = new_container;
  containers.list.array = (V**)realloc(containers.list.array, ++containers.list.size * sizeof(V *));
  containers.list.count++;

}

//case for containerId = 5
template<typename T,typename V>
void add_new_item_to_container(T &containers,long containerid_val, PerceivedObjectContainer &perobcon)
{
  V* new_container = (V*)malloc(sizeof(V));
  
  //PerceivedObjectContainer *w;
  //w->numberOfPerceivedObjects

  //w->containerData.choice.PerceivedObjectContainer.perceivedObjects
  //new_container->containerData.choice.PerceivedObjectContainer = (PerceivedObjectContainer_t*)malloc(sizeof(PerceptionRegionContainer_t));

  new_container->containerId = containerid_val;
  new_container->containerData.choice.PerceivedObjectContainer.perceivedObjects.list = perobcon.perceivedObjects.list;
  new_container->containerData.choice.PerceivedObjectContainer.numberOfPerceivedObjects = perobcon.numberOfPerceivedObjects;

  std::cout << "SUCCESS" << std::endl;
  
  containers.list.array[containers.list.count] = new_container;
  containers.list.array = (V**)realloc(containers.list.array, ++containers.list.size * sizeof(V *));
  containers.list.count++;

}







namespace ns3 {

  NS_LOG_COMPONENT_DEFINE("CPBasicService");

  CPBasicService::CPBasicService()
  {
    m_station_id = ULONG_MAX;
    m_stationtype = LONG_MAX;
    m_socket_tx=NULL;
    m_btp = NULL;
    m_LDM = NULL;
    m_real_time=false;

    // Setting a default value of m_T_CheckCpmGen_ms equal to 100 ms (i.e. T_GenCpmMin_ms)
    m_T_CheckCpmGen_ms=T_GenCpmMin_ms;

    m_prev_heading=-1;
    m_prev_speed=-1;
    m_prev_distance=-1;

    m_T_GenCpm_ms=T_GenCpmMax_ms;

    lastCpmGen=-1;
    lastCpmGenLowFrequency=-1;
    lastCpmGenSpecialVehicle=-1;

    m_T_LastSensorInfoContainer = -1;

    m_N_GenCpmMax=1000;
    m_N_GenCpm=100;

    m_vehicle=true;
    m_redundancy_mitigation = true;

    m_cpm_sent=0;
  }


  
  void
  CPBasicService::setStationID(unsigned long fixed_stationid)
  {
    m_station_id=fixed_stationid;
    m_btp->setStationID(fixed_stationid);
  }

  void
  CPBasicService::setStationType(long fixed_stationtype)
  {
    m_stationtype=fixed_stationtype;
    m_btp->setStationType(fixed_stationtype);
  }

  void
  CPBasicService::setStationProperties(unsigned long fixed_stationid,long fixed_stationtype)
  {
    m_station_id=fixed_stationid;
    m_stationtype=fixed_stationtype;
    m_btp->setStationProperties(fixed_stationid,fixed_stationtype);
  }

  void
  CPBasicService::setSocketRx (Ptr<Socket> socket_rx)
  {
    m_btp->setSocketRx(socket_rx);
    m_btp->addCPMRxCallback (std::bind(&CPBasicService::receiveCpm,this,std::placeholders::_1,std::placeholders::_2));
  }

  void
  CPBasicService::initDissemination()
  {
    std::cout << "New Call Diss" << std::endl;
    empty_json_file(filename);
    std::srand(Simulator::Now().GetNanoSeconds ());
    double desync = ((double)std::rand()/RAND_MAX);
    m_event_cpmSend = Simulator::Schedule (Seconds(desync), &CPBasicService::generateAndEncodeCPM, this);
  }

  double
  CPBasicService::cartesian_dist(double lon1, double lat1, double lon2, double lat2)
  {
    libsumo::TraCIPosition pos1,pos2;
    pos1 = m_client->TraCIAPI::simulation.convertLonLattoXY(lon1,lat1);
    pos2 = m_client->TraCIAPI::simulation.convertLonLattoXY(lon2,lat2);
    return sqrt((pow((pos1.x-pos2.x),2)+pow((pos1.y-pos2.y),2)));
  }
  bool
  CPBasicService::checkCPMconditions(std::vector<LDM::returnedVehicleData_t>::iterator PO_data)
  {
    /*Perceived Object Container Inclusion Management as mandated by TS 103 324 Section 6.1.2.3*/
    std::map<uint64_t, PHData_t> phPoints = PO_data->phData.getPHpoints ();
    PHData_t previousCPM;
    /* 1.a The object has first been detected by the perception system after the last CPM generation event.*/
    if((PO_data->phData.getSize ()==1) && (PO_data->phData.getPHpoints ().begin ()->first > lastCpmGen))
      return true;

    /* Get the last position of the reference point of this object lastly included in a CPM from the object pathHistory*/
    std::map<uint64_t, PHData_t>::reverse_iterator it = phPoints.rbegin ();
    it ++;
    for(auto fromPrev = it; fromPrev!=phPoints.rend(); fromPrev++)
      {
        if (fromPrev->second.CPMincluded == true)
          {
            previousCPM = fromPrev->second;
          }
      }
    /* 1.b The Euclidian absolute distance between the current estimated position of the reference point of the
     * object and the estimated position of the reference point of this object lastly included in a CPM exceeds
     * 4 m. */
    if(m_vdp->getCartesianDist (previousCPM.lon,previousCPM.lat,PO_data->vehData.lon,PO_data->vehData.lat) > 4.0)
      return true;
    /* 1.c The difference between the current estimated absolute speed of the reference point of the object and the
     * estimated absolute speed of the reference point of this object lastly included in a CPM exceeds 0,5 m/s. */
    if(abs(previousCPM.speed_ms - PO_data->vehData.speed_ms) > 0.5)
      return true;
    /* 1.d The difference between the orientation of the vector of the current estimated absolute velocity of the
     * reference point of the object and the estimated orientation of the vector of the absolute velocity of the
     * reference point of this object lastly included in a CPM exceeds 4 degrees. */
    if(abs(previousCPM.heading - PO_data->vehData.heading) > 4)
      return true;
    /* 1.e The time elapsed since the last time the object was included in a CPM exceeds T_GenCpmMax. */
    if(PO_data->vehData.lastCPMincluded.isAvailable ())
      {
        if(PO_data->vehData.lastCPMincluded.getData() < ((computeTimestampUInt64 ()/NANO_TO_MILLI)-m_N_GenCpmMax))
          return true;
      }
    return false;
  }

  void
  CPBasicService::generateAndEncodeCPM()
  {
    VDP::CPM_mandatory_data_t cpm_mandatory_data;
    Ptr<Packet> packet;

    BTPDataRequest_t dataRequest = {};

    int64_t now = computeTimestampUInt64 () / NANO_TO_MILLI;

    std::string encode_result;

    long numberOfPOs = 0;
    long container_counter = 1;

    //for json file
    const std::string filename = "/tmp/cpm.json";

    /* Collect data for mandatory containers */
    auto cpm = asn1cpp::makeSeq (CollectivePerceptionMessage);

    if (bool (cpm) == false)
      {
        NS_LOG_ERROR ("Warning: unable to encode CPM.");
        return;
      }

    //Schedule new CPM
    m_event_cpmSend = Simulator::Schedule (MilliSeconds (m_N_GenCpm),
                                           &CPBasicService::generateAndEncodeCPM, this);

    auto CPMcontainers = asn1cpp::makeSeq (WrappedCpmContainers);
    auto POsContainer = asn1cpp::makeSeq (PerceivedObjectContainer);
    auto CPM_POs = asn1cpp::makeSeq (PerceivedObjects);
    std::cout << "[CPM] Vehicle " << m_station_id << " position: " << m_vdp->getPositionXY().x << " " << m_vdp->getPositionXY().y << " and time " << now<< std::endl;
    
    //std::cout << "LDM IS" << m_LDM << " of Vehicle" << m_station_id << std::endl;

    CollectivePerceptionMessage *cpm1;
    cpm1 = (CollectivePerceptionMessage*)malloc(sizeof(struct CollectivePerceptionMessage));
    cpm1->payload.cpmContainers.list.array = (struct WrappedCpmContainer **)malloc(sizeof(struct WrappedCpmContainer *));

    WrappedCpmContainers wrcpms;
  
    wrcpms.list.array = (struct WrappedCpmContainer **)malloc(sizeof(struct WrappedCpmContainer *));
    wrcpms.list.count = 0;
    wrcpms.list.size = 1;

    //cpm1.payload.cpmContainers.list.free(&wrcpm1);

    PerceivedObjectContainer *perobjcont;
    perobjcont = (PerceivedObjectContainer*)malloc(sizeof(struct PerceivedObjectContainer));
    perobjcont->perceivedObjects.list.array = (struct PerceivedObject **)malloc(sizeof(struct PerceivedObject *));

    PerceivedObjects perobjs;

    perobjs.list.array = (struct PerceivedObject **)malloc(sizeof(struct PerceivedObject *));
    perobjs.list.count = 0;
    perobjs.list.size = 1;

    if (m_LDM != NULL)
      {
        std::vector<LDM::returnedVehicleData_t> LDM_POs;
        //std::cout << "LDM OF VEHICLE " << m_station_id << " IS NOT NULL" << std::endl;
        //std::cout << "POs FLAG OF " << m_station_id << " IS " << bool ( m_LDM->getAllPOs (LDM_POs) ) << std::endl;
        if (m_LDM->getAllPOs (LDM_POs)) // If there are any POs in the LDM
          {
            std::cout << "THERE ARE POs in the VEHICLE " << m_station_id << std::endl;
            /* Fill Perceived Object Container as detailed in ETSI TS 103 324, Section 7.1.8 */
            std::vector<LDM::returnedVehicleData_t>::iterator it;
            for (it = LDM_POs.begin (); it != LDM_POs.end (); it++)
              {

                if (it->vehData.perceivedBy.getData () != (long) m_station_id)
                  continue;
                if (!checkCPMconditions (it) && m_redundancy_mitigation)
                  continue;
                else
                  {
                    auto PO = asn1cpp::makeSeq (PerceivedObject);
                    asn1cpp::setField (PO->objectId, it->vehData.stationID);
                    std::cout << "ID: " << it->vehData.ID << " and StationID: " << it->vehData.stationID << std::endl;
                    long timeOfMeasurement =
                        (Simulator::Now ().GetMicroSeconds () - it->vehData.timestamp_us) /
                        1000; // time of measuremente in ms
                    if (timeOfMeasurement > 1500)
                      timeOfMeasurement = 1500;
                    asn1cpp::setField (PO->measurementDeltaTime, timeOfMeasurement);
                    //std::cout << "DELTA TIME " << PO->measurementDeltaTime << std::endl;
                    asn1cpp::setField (PO->position.xCoordinate.value,
                                       it->vehData.xDistAbs.getData ());
                    //std::cout << "POSITION X OF IT ITEM: " << it->vehData.xDistAbs.getData () << std::endl;
                    asn1cpp::setField (PO->position.xCoordinate.confidence,
                                       CoordinateConfidence_unavailable);
                    asn1cpp::setField (PO->position.yCoordinate.value,
                                       it->vehData.yDistAbs.getData ());
                    asn1cpp::setField (PO->position.yCoordinate.confidence,
                                       CoordinateConfidence_unavailable);

                    auto velocity = asn1cpp::makeSeq (Velocity3dWithConfidence);
                    asn1cpp::setField (velocity->present,
                                       Velocity3dWithConfidence_PR_cartesianVelocity);
                    auto cartesianVelocity = asn1cpp::makeSeq (VelocityCartesian);
                    asn1cpp::setField (cartesianVelocity->xVelocity.value,
                                       it->vehData.xSpeedAbs.getData ());
                    //std::cout << "VELOCITY X OF IT ITEM: " << it->vehData.xSpeedAbs.getData () << std::endl;
                    asn1cpp::setField (cartesianVelocity->xVelocity.confidence,
                                       SpeedConfidence_unavailable);
                    asn1cpp::setField (cartesianVelocity->yVelocity.value,
                                       it->vehData.ySpeedAbs.getData ());
                    asn1cpp::setField (cartesianVelocity->yVelocity.confidence,
                                       SpeedConfidence_unavailable);
                    asn1cpp::setField (velocity->choice.cartesianVelocity, cartesianVelocity);
                    asn1cpp::setField (PO->velocity, velocity);

                    auto acceleration = asn1cpp::makeSeq (Acceleration3dWithConfidence);
                    asn1cpp::setField (acceleration->present,
                                       Acceleration3dWithConfidence_PR_cartesianAcceleration);
                    auto cartesianAcceleration = asn1cpp::makeSeq (AccelerationCartesian);
                    asn1cpp::setField (cartesianAcceleration->xAcceleration.value,
                                       it->vehData.xAccAbs.getData ());
                    asn1cpp::setField (cartesianAcceleration->xAcceleration.confidence,
                                       AccelerationConfidence_unavailable);
                    asn1cpp::setField (cartesianAcceleration->yAcceleration.value,
                                       it->vehData.yAccAbs.getData ());
                    asn1cpp::setField (cartesianAcceleration->yAcceleration.confidence,
                                       AccelerationConfidence_unavailable);
                    asn1cpp::setField (acceleration->choice.cartesianAcceleration,
                                       cartesianAcceleration);
                    asn1cpp::setField (PO->acceleration, acceleration);
                    //std::cout << "ACCELERATION: " << PO->acceleration->choice.cartesianAcceleration.xAcceleration.value << std::endl;

                    //Only z angle
                    auto angle = asn1cpp::makeSeq (EulerAnglesWithConfidence);
                    if ((it->vehData.heading*DECI) < CartesianAngleValue_unavailable &&
                        (it->vehData.heading*DECI) > 0)
                      asn1cpp::setField (angle->zAngle.value, (it->vehData.heading*DECI));
                    else
                      asn1cpp::setField (angle->zAngle.value, CartesianAngleValue_unavailable);
                    asn1cpp::setField (angle->zAngle.confidence, AngleConfidence_unavailable);
                    asn1cpp::setField (PO->angles, angle);
                    auto OD1 = asn1cpp::makeSeq (ObjectDimension);
                    if (it->vehData.vehicleLength.getData () < 1023 &&
                        it->vehData.vehicleLength.getData () > 0)
                      asn1cpp::setField (OD1->value, it->vehData.vehicleLength.getData ());
                    else
                      asn1cpp::setField (OD1->value, 50); //usual value for SUMO vehicles
                    asn1cpp::setField (OD1->confidence, ObjectDimensionConfidence_unavailable);
                    asn1cpp::setField (PO->objectDimensionX, OD1);
                    auto OD2 = asn1cpp::makeSeq (ObjectDimension);
                    if (it->vehData.vehicleWidth.getData () < 1023 &&
                        it->vehData.vehicleWidth.getData () > 0)
                      asn1cpp::setField (OD2->value, it->vehData.vehicleWidth.getData ());
                    else
                      asn1cpp::setField (OD2->value, 18); //usual value for SUMO vehicles
                    asn1cpp::setField (OD2->confidence, ObjectDimensionConfidence_unavailable);
                    asn1cpp::setField (PO->objectDimensionY, OD2);

                    /*Rest of optional fields handling left as future work*/
                    
                    //std::cout << "POSITION X OF PO: " << PO->position.xCoordinate.value << std::endl;

                    //Push Perceived Object to the container
                    asn1cpp::sequenceof::pushList (*CPM_POs, PO);

                    add_new_item_to_container<PerceivedObjects,PerceivedObject>(perobjs,PO->objectId,PO->measurementDeltaTime,\
                    PO->position,PO->velocity,PO->acceleration,PO->angles,PO->objectDimensionX,PO->objectDimensionY);



                    //present CPM info into a json format
                    /*
                    PerceivedObject PO1;

                    PO1.measurementDeltaTime = timeOfMeasurement;
                    PO1.position.xCoordinate.value = it->vehData.xDistAbs.getData () ;
                    PO1.position.xCoordinate.value = it->vehData.xDistAbs.getData () ;
                    PO1.position.xCoordinate.confidence =  CoordinateConfidence_unavailable;
                    PO1.position.yCoordinate.confidence =  CoordinateConfidence_unavailable;

                    struct_mapping::reg(&PerceivedObject::measurementDeltaTime, "delta_time");

                    struct_mapping::reg(&CartesianCoordinateWithConfidence::value, "value");
                    struct_mapping::reg(&CartesianCoordinateWithConfidence::confidence, "confidence");

                    struct_mapping::reg(&CartesianPosition3dWithConfidence::xCoordinate, "xCoordinate");
                    struct_mapping::reg(&CartesianPosition3dWithConfidence::yCoordinate, "yCoordinate");

                    struct_mapping::reg(&PerceivedObject::position, "position");
                    //Velocity3dWithConfidence velocity1;

                    //velocity1.present = Velocity3dWithConfidence_PR_cartesianVelocity;
                    //velocity1.choice.cartesianVelocity.xVelocity.value =  it->vehData.xSpeedAbs.getData ();
                    //velocity1.choice.cartesianVelocity.yVelocity.value = it->vehData.ySpeedAbs.getData ();

                    struct_mapping::reg(&VelocityComponent::value, "value");
                    //struct_mapping::reg(&VelocityComponent::confidence, "confidence");

                    struct_mapping::reg(&VelocityCartesian::xVelocity, "xVelocity");
                    struct_mapping::reg(&VelocityCartesian::yVelocity, "yVelocity");

                    struct_mapping::reg(&Velocity3dWithConfidence::present, "present");
                    struct_mapping::reg(&Velocity3dWithConfidence::choice, "choice");
                   
                    struct_mapping::reg(&PerceivedObject::velocity, "velocity");
                    std::ostringstream json_data;
                    
                    struct_mapping::map_struct_to_json(PO1, json_data, "  ");

                    //struct_mapping::map_struct_to_json(velocity1, json_data, "  ");

                    std::cout << json_data.str() << std::endl;
                    */

                    //Update the timestamp of the last time this PO was included in a CPM
                    m_LDM->updateCPMincluded (it->vehData.stationID,
                                              computeTimestampUInt64 () / NANO_TO_MILLI);
                    //Increase number of POs for the numberOfPerceivedObjects field in cpmParameters container
                    numberOfPOs++;
                    //std::cout << "NUMBER IS " << numberOfPOs <<std::endl;
                  }
              }
            if (numberOfPOs != 0)
              {
                asn1cpp::setField (POsContainer->perceivedObjects, CPM_POs);
                asn1cpp::setField (POsContainer->numberOfPerceivedObjects, numberOfPOs);
                perobjcont->numberOfPerceivedObjects = numberOfPOs;
                perobjcont->perceivedObjects.list = perobjs.list;

              }
          }
      }

    /* Fill the header */
    asn1cpp::setField (cpm->header.messageId, MessageId_cpm);
    asn1cpp::setField (cpm->header.protocolVersion, 2);
    asn1cpp::setField (cpm->header.stationId, m_station_id);

    //CPM STRUCT TO JSON

    //CollectivePerceptionMessage cpm1;
    json j;

    cpm1->header.messageId = MessageId_cpm;
    cpm1->header.protocolVersion = 2;
    cpm1->header.stationId = m_station_id;
    
    /*
    struct_mapping::reg(&ItsPduHeader::messageId, "messageId");
    struct_mapping::reg(&ItsPduHeader::protocolVersion, "protocolVersion");
    struct_mapping::reg(&ItsPduHeader::stationId, "stationId");
   
    struct_mapping::reg(&CollectivePerceptionMessage::header, "header");

    struct_mapping::reg(&WrappedCpmContainers::list, "WrappedCpmContainersList");
    struct_mapping::reg(&WrappedCpmContainer::containerId, "containerId");
    struct_mapping::reg(&CpmPayload::cpmContainers, "cpmContainers");

    struct_mapping::reg(&CollectivePerceptionMessage::payload, "payload");

    std::ostringstream json_data;
    struct_mapping::map_struct_to_json(cpm, json_data, "  ");
    
    std::cout << json_data.str() << std::endl;
    */

    /*
     * Compute the generationDeltaTime, "computed as the time corresponding to the
     * time of the reference position in the CPM, considered as time of the CPM generation.
     * The value of the generationDeltaTime shall be wrapped to 65 536. This value shall be set as the
     * remainder of the corresponding value of TimestampIts divided by 65 536 as below:
     * generationDeltaTime = TimestampIts mod 65 536"
    */
    asn1cpp::setField (cpm->payload.managementContainer.referenceTime,
                       compute_timestampIts (m_real_time) % 65536);

    long genDeltaTime = compute_timestampIts (m_real_time) % 65536;

    cpm_mandatory_data = m_vdp->getCPMMandatoryData ();

    /* Fill the managementContainer */
    asn1cpp::setField (cpm->payload.managementContainer.referencePosition.altitude.altitudeValue,
                       cpm_mandatory_data.altitude.getValue ());
    asn1cpp::setField (
        cpm->payload.managementContainer.referencePosition.altitude.altitudeConfidence,
        cpm_mandatory_data.altitude.getConfidence ());
    asn1cpp::setField (cpm->payload.managementContainer.referencePosition.latitude,
                       cpm_mandatory_data.latitude);
    asn1cpp::setField (cpm->payload.managementContainer.referencePosition.longitude,
                       cpm_mandatory_data.longitude);
    asn1cpp::setField (cpm->payload.managementContainer.referencePosition.positionConfidenceEllipse
                           .semiMajorConfidence,
                       cpm_mandatory_data.posConfidenceEllipse.semiMajorConfidence);
    asn1cpp::setField (cpm->payload.managementContainer.referencePosition.positionConfidenceEllipse
                           .semiMinorConfidence,
                       cpm_mandatory_data.posConfidenceEllipse.semiMinorConfidence);
    asn1cpp::setField (cpm->payload.managementContainer.referencePosition.positionConfidenceEllipse
                           .semiMajorOrientation,
                       cpm_mandatory_data.posConfidenceEllipse.semiMajorOrientation);
    //TODO:  compute segmentInfo, get MTU and deal with needed segmentation

    /* Fill the originatingVehicleContainer */
    auto wrappedCpmContainer = asn1cpp::makeSeq (WrappedCpmContainer);
    asn1cpp::setField (wrappedCpmContainer->containerId, 1);
    auto originatingVehicleContainer = asn1cpp::makeSeq (OriginatingVehicleContainer);
    asn1cpp::setField (originatingVehicleContainer->orientationAngle.value,
                       cpm_mandatory_data.heading.getValue ());
    asn1cpp::setField (originatingVehicleContainer->orientationAngle.confidence,
                       cpm_mandatory_data.heading.getConfidence ());
    asn1cpp::setField (wrappedCpmContainer->containerData.present,
                       WrappedCpmContainer__containerData_PR_OriginatingVehicleContainer);
    asn1cpp::setField (wrappedCpmContainer->containerData.choice.OriginatingVehicleContainer,
                       originatingVehicleContainer);
    asn1cpp::sequenceof::pushList (cpm->payload.cpmContainers, wrappedCpmContainer);

    //CPM STRUCT TO JSON
    
    cpm1->payload.managementContainer.referencePosition.latitude = cpm_mandatory_data.latitude;
    cpm1->payload.managementContainer.referencePosition.longitude = cpm_mandatory_data.longitude;
    cpm1->payload.managementContainer.referencePosition.longitude = cpm_mandatory_data.altitude.getValue ();
    
  

    /* Generate Sensor Information Container as detailed in ETSI TS 103 324, Section 6.1.2.2 */
    if (now - m_T_LastSensorInfoContainer >= m_T_AddSensorInformation)
      {
        //std::cout << "IN SECOND WRAPCONT CASE" << std::endl;
        auto CPMcontainer = asn1cpp::makeSeq (WrappedCpmContainer);
        asn1cpp::setField (CPMcontainer->containerId, 3);
        auto sensorInfoContainer = asn1cpp::makeSeq (SensorInformationContainer);

        //For now we only consider one sensor
        //We assume sensor fusion or aggregation of 50m sensing range from the vehicle front bumper
        auto sensorInfo = asn1cpp::makeSeq (SensorInformation);
        asn1cpp::setField (sensorInfo->sensorId, 2);
        asn1cpp::setField (sensorInfo->sensorType, SensorType_itsAggregation);
        asn1cpp::setField (sensorInfo->shadowingApplies, true);
        auto detectionArea = asn1cpp::makeSeq (Shape);
        asn1cpp::setField (detectionArea->present, Shape_PR_circular);
        auto circularArea = asn1cpp::makeSeq (CircularShape);
        auto egoPos = m_vdp->getPositionXY ();
        auto refPos = asn1cpp::makeSeq (CartesianPosition3d);
        asn1cpp::setField (refPos->xCoordinate, egoPos.x);
        asn1cpp::setField (refPos->yCoordinate, egoPos.y);
        asn1cpp::setField (circularArea->shapeReferencePoint, refPos);
        asn1cpp::setField (circularArea->radius, 50);
        asn1cpp::setField (detectionArea->choice.circular, circularArea);
        asn1cpp::setField (sensorInfo->perceptionRegionShape, detectionArea);

        asn1cpp::sequenceof::pushList (*sensorInfoContainer, sensorInfo);

        asn1cpp::setField (CPMcontainer->containerData.present,
                           WrappedCpmContainer__containerData_PR_SensorInformationContainer);
        asn1cpp::setField (CPMcontainer->containerData.choice.SensorInformationContainer,
                           sensorInfoContainer);
        asn1cpp::sequenceof::pushList (cpm->payload.cpmContainers, CPMcontainer);
        m_T_LastSensorInfoContainer = now;
        
        //WrappedCpmContainer* wrcpm2 = (WrappedCpmContainer*)malloc(sizeof(WrappedCpmContainer));
        //WrappedCpmContainer* wrcpm2 = add_new_wrcpm(wrcpms,CPMcontainer->containerId);
        
        //add_new_wrcpm(wrcpms,CPMcontainer->containerId);

        SensorInformationContainer sensorcont;
        
        sensorcont.list.array = (struct SensorInformation **)malloc(sizeof(struct SensorInformation *));
        sensorcont.list.count = 0;
        sensorcont.list.size = 1;
      
        //SensorInformation* sen = (SensorInformation*) malloc(sizeof(struct SensorInformation *));
        //std::cout << "OLALALA" << std::endl;
        //sen->perceptionRegionShape->choice.circular.radius = circularArea->radius;
        //free(sen);

        //std::cout << "OLALA" << std::endl;

        add_new_item_to_container<WrappedCpmContainers,WrappedCpmContainer>(wrcpms,CPMcontainer->containerId,sensorcont,\
        sensorInfo->sensorId,sensorInfo->sensorType,sensorInfo->shadowingApplies,detectionArea->present,circularArea->radius,\
        refPos->xCoordinate,refPos->yCoordinate);
        
        //wrcpm2.containerData.choice.OriginatingVehicleContainer.orientationAngle.value = cpm_mandatory_data.heading.getValue ();
        //wrcpm2->containerId = CPMcontainer->containerId;
        //std::cout << "IDCONT: " << wrcpm2->containerId << std::endl;

        //wrcpms.list.array[wrcpms.list.count] = wrcpm2;
        //wrcpms.list.array = (struct WrappedCpmContainer **)realloc(wrcpms.list.array, ++wrcpms.list.size * sizeof(struct WrappedCpmContainer *));
        //wrcpms.list.count++;
        
        std::cout << "COUNTER1: " <<wrcpms.list.count << std::endl;

      }
    else
      {
        //If no sensorInformationContainer and no perceivedObjectsContainer
        if (numberOfPOs == 0)
          return; //No CPM is generated in the current cycle
      }

    if (numberOfPOs != 0)
      {
        auto CPMcontainer = asn1cpp::makeSeq (WrappedCpmContainer);
        asn1cpp::setField (CPMcontainer->containerId, 5);
        asn1cpp::setField (CPMcontainer->containerData.present,
                           WrappedCpmContainer__containerData_PR_PerceivedObjectContainer);
        asn1cpp::setField (CPMcontainer->containerData.choice.PerceivedObjectContainer,
                           POsContainer);
        asn1cpp::sequenceof::pushList(cpm->payload.cpmContainers,CPMcontainer);
        
        //add_new_wrcpm(wrcpms,CPMcontainer->containerId);
        add_new_item_to_container<WrappedCpmContainers,WrappedCpmContainer>(wrcpms,CPMcontainer->containerId,*perobjcont);
      }

    //WrappedCpmContainer* wrcpm1 = add_new_wrcpm(wrcpms,wrappedCpmContainer->containerId);

    //add_new_wrcpm(wrcpms,wrappedCpmContainer->containerId);
    add_new_item_to_container<WrappedCpmContainers,WrappedCpmContainer>(wrcpms,wrappedCpmContainer->containerId,\
    cpm_mandatory_data.heading.getValue (),cpm_mandatory_data.heading.getConfidence ());
    //wrcpms.list.array[ wrcpms.list.count-1]->containerData.choice.OriginatingVehicleContainer.orientationAngle.value = cpm_mandatory_data.heading.getValue ();
    //wrcpm1->containerData.choice.OriginatingVehicleContainer.orientationAngle.value = cpm_mandatory_data.heading.getValue ();
    //wrcpm1->containerId = wrappedCpmContainer->containerId;

    //wrcpms.list.array[wrcpms.list.count] = wrcpm1;
    //wrcpms.list.array = (struct WrappedCpmContainer **)realloc(wrcpms.list.array, ++wrcpms.list.size * sizeof(struct WrappedCpmContainer *));
    /*if (wrcpms.list.count == 1)
    {
      std::cout << "IDCONT2: " << wrcpms.list.array[ wrcpms.list.count-1]->containerId << std::endl;
    }*/
    //wrcpms.list.count++;

    cpm1->payload.cpmContainers.list = wrcpms.list;

    //std::cout << "COUNTER2: " <<wrcpms.list.count << std::endl;

    //std::cout << "COUNTER3: " <<cpm1->payload.cpmContainers.list.count << std::endl;

    to_json(j,*cpm1,filename,genDeltaTime);

    //std::cout << j.dump(4) << std::endl;

    /*if (numberOfPOs!=0){
      std::cout << "PER COUNTER " << perobjcont->perceivedObjects.list.count << std::endl;
      for(int i=0; i<perobjcont->perceivedObjects.list.count; i++)
      {
        //std::cout << "REACHED HERE PER " << std::endl;
        free(perobjcont->perceivedObjects.list.array[i]->objectId);
        free(perobjcont->perceivedObjects.list.array[i]->velocity);
        free(perobjcont->perceivedObjects.list.array[i]->acceleration);
        free(perobjcont->perceivedObjects.list.array[i]->angles);
        free(perobjcont->perceivedObjects.list.array[i]->objectDimensionX);
        free(perobjcont->perceivedObjects.list.array[i]->objectDimensionY);
        free(perobjcont->perceivedObjects.list.array[i]);
      }
      free(perobjcont->perceivedObjects.list.array);
      free(perobjcont);
      std::cout << "END UP HERE 1" << std::endl;
    }*/

    for(int i=0; i<cpm1->payload.cpmContainers.list.count; i++) 
    {
      //std::cout << "REACHED HERE " << std::endl;
        
      switch(cpm1->payload.cpmContainers.list.array[i]->containerId)
      {
        case 3:
          for(int j=0; j<cpm1->payload.cpmContainers.list.array[i]->containerData.choice.SensorInformationContainer.list.count; j++)
          {
            //std::cout << "REACHED HERE 2" << std::endl;
            free(cpm1->payload.cpmContainers.list.array[i]->containerData.choice.SensorInformationContainer.list.array[j]->perceptionRegionShape->\
            choice.circular.shapeReferencePoint);
            std::cout << "END UP HERE 2" << std::endl;
            free(cpm1->payload.cpmContainers.list.array[i]->containerData.choice.SensorInformationContainer.list.array[j]->perceptionRegionShape);
            free(cpm1->payload.cpmContainers.list.array[i]->containerData.choice.SensorInformationContainer.list.array[j]);
          }
          break;
        case 5:
          std::cout << "PER COUNTER " << cpm1->payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.count << std::endl;
          for(int j=0; j<cpm1->payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.count; j++)
          {
            //std::cout << "REACHED HERE PER " << std::endl;
            free(cpm1->payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[j]->objectId);
            free(cpm1->payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[j]->velocity);
            free(cpm1->payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[j]->acceleration);
            free(cpm1->payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[j]->angles);
            free(cpm1->payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[j]->objectDimensionX);
            free(cpm1->payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[j]->objectDimensionY);
            free(cpm1->payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array[j]);
          }
          free(cpm1->payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects.list.array);
          //free(cpm1->payload.cpmContainers.list.array[i]->containerData.choice.PerceivedObjectContainer.perceivedObjects);
          std::cout << "END UP HERE 5" << std::endl;
          free(perobjcont);
          std::cout << "END UP HERE 6" << std::endl;
          break;
      }
        
      free(cpm1->payload.cpmContainers.list.array[i]);
    }
    std::cout << "END UP HERE 3" << std::endl;
    free(cpm1->payload.cpmContainers.list.array);
    free(cpm1);

    std::cout << "END UP HERE 4" << std::endl;
    // TODO: Support for Perception Region information from LDM (to be implemented in both SUMOensor and CARLAsensor)

    encode_result = asn1cpp::uper::encode(cpm);
    if(encode_result.size()<1)
    {
        NS_LOG_ERROR("Warning: unable to encode CPM.");
        return;
    }

    packet = Create<Packet> ((uint8_t*) encode_result.c_str(), encode_result.size());
    //packet = Create<Packet> ((uint8_t*) bytes, length);

    dataRequest.BTPType = BTP_B; //!< BTP-B
    dataRequest.destPort = CP_PORT;
    dataRequest.destPInfo = 0;
    dataRequest.GNType = TSB;
    dataRequest.GNCommProfile = UNSPECIFIED;
    dataRequest.GNRepInt =0;
    dataRequest.GNMaxRepInt=0;
    dataRequest.GNMaxLife = 1;
    dataRequest.GNMaxHL = 1;
    dataRequest.GNTraClass = 0x02; // Store carry foward: no - Channel offload: no - Traffic Class ID: 2
    dataRequest.lenght = packet->GetSize ();
    dataRequest.data = packet;
    m_btp->sendBTP(dataRequest);

    m_cpm_sent++;

    // Store the time in which the last CPM (i.e. this one) has been generated and successfully sent
    m_T_GenCpm_ms=now-lastCpmGen;
    lastCpmGen = now;
  }
  void
  CPBasicService::startCpmDissemination()
  {
    // Old desync code kept just for reference
    // It may lead to nodes not being desynchronized properly in specific situations in which
    // Simulator::Now().GetNanoSeconds () returns the same seed for multiple nodes
    // std::srand(Simulator::Now().GetNanoSeconds ());
    // double desync = ((double)std::rand()/RAND_MAX);

    Ptr<UniformRandomVariable> desync_rvar = CreateObject<UniformRandomVariable> ();
    desync_rvar->SetAttribute ("Min", DoubleValue (0.0));
    desync_rvar->SetAttribute ("Max", DoubleValue (1.0));
    double desync = desync_rvar->GetValue ();
    m_event_cpmDisseminationStart = Simulator::Schedule (Seconds(desync), &CPBasicService::initDissemination, this);
  }

  uint64_t
  CPBasicService::terminateDissemination()
  {

    Simulator::Remove(m_event_cpmDisseminationStart);
    Simulator::Remove(m_event_cpmSend);
    return m_cpm_sent;
    
  }

  void
  CPBasicService::receiveCpm (BTPDataIndication_t dataIndication, Address from)
  {
    Ptr<Packet> packet;
    asn1cpp::Seq<CollectivePerceptionMessage> decoded_cpm,cpm_test;

    uint8_t *buffer; //= new uint8_t[packet->GetSize ()];
    buffer=(uint8_t *)malloc((dataIndication.data->GetSize ())*sizeof(uint8_t));
    dataIndication.data->CopyData (buffer, dataIndication.data->GetSize ());
    std::string packetContent((char *)buffer,(int) dataIndication.data->GetSize ());

    RssiTag rssi;
    bool rssi_result = dataIndication.data->PeekPacketTag(rssi);

    SnrTag snr;
    bool snr_result = dataIndication.data->PeekPacketTag(snr);

    RsrpTag rsrp;
    bool rsrp_result = dataIndication.data->PeekPacketTag(rsrp);

    SinrTag sinr;
    bool sinr_result = dataIndication.data->PeekPacketTag(sinr);

    SizeTag size;
    bool size_result = dataIndication.data->PeekPacketTag(size);

    TimestampTag timestamp;
    dataIndication.data->PeekPacketTag(timestamp);

    if(!snr_result)
      {
        snr.Set(SENTINEL_VALUE);
      }
    if (!rssi_result)
      {
        rssi.Set(SENTINEL_VALUE);
      }
    if (!rsrp_result)
      {
        rsrp.Set(SENTINEL_VALUE);
      }
    if (!sinr_result)
      {
        sinr.Set(SENTINEL_VALUE);
      }
    if (!size_result)
      {
        size.Set(SENTINEL_VALUE);
      }

    SetSignalInfo(timestamp.Get(), size.Get(), rssi.Get(), snr.Get(), sinr.Get(), rsrp.Get());


    /** Decoding **/
    decoded_cpm = asn1cpp::uper::decode(packetContent, CollectivePerceptionMessage);

    if(bool(decoded_cpm)==false) {
        NS_LOG_ERROR("Warning: unable to decode a received CPM.");
        return;
      }

    m_CPReceiveCallback(decoded_cpm,from);
  }
  int64_t
  CPBasicService::computeTimestampUInt64()
  {
    int64_t int_tstamp=0;

    if (!m_real_time)
      {
        int_tstamp=Simulator::Now ().GetNanoSeconds ();
      }
    else
      {
        struct timespec tv;

        clock_gettime (CLOCK_MONOTONIC, &tv);

        int_tstamp=tv.tv_sec*1e9+tv.tv_nsec;
      }
    return int_tstamp;
  }
}
