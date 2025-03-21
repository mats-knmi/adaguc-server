#include <vector>
#include <algorithm>
#include "CMakeEProfile.h"
#include "CImageDataWriter.h"
#include "CUniqueRequests/CURTypes.h"

const char *CMakeEProfile::className = "CMakeEProfile";

// #define CMakeEProfile_DEBUG

#define CMakeEProfile_MAX_DIMS 255

#define DEFAULT_VALIDITY_LENGTH_OF_OBSERVATION_IN_SECONDS 12.0f

std::string encodeJSON(CT::string input) {
  CT::string str = input;
  str.replaceSelf("\"", "\\");
  str.replaceSelf("\n", "");
  str.replaceSelf("\"", "\\\"");
  return str.c_str();
}

class EProfileUniqueRequests {
private:
  DEF_ERRORFUNCTION();

public:
  bool readDataAsCDFDouble;

  class AggregatedDimension {
  public:
    CT::string name;
    int start;
    std::vector<std::string> values;
  };

  struct DimInfo {
    std::map<int, std::string> dimValuesMap;             // All values, many starts with 1 count, result of set()
    std::vector<AggregatedDimension *> aggregatedValues; // Aggregated values (start/count series etc), result of  addDimSet()
  };

  typedef std::map<std::string, DimInfo *>::iterator it_type_diminfo;

  class Request {
  public:
    int numDims;
    AggregatedDimension *dimensions[CMakeEProfile_MAX_DIMS];
  };

  int drawEprofile(CDrawImage *drawImage, CDF::Variable *variable, size_t *start, size_t *count, EProfileUniqueRequests::Request *, CDataSource *dataSource, CT::string *eProfileJSON);
  int plotHeightRetrieval(CDrawImage *drawImage, CDFObject *cdfObject, const char *varName, CColor c, size_t NrOfDates, double startGraphTime, double startGraphRange, double graphWidth,
                          double graphHeight, int timeWidth);

  class FileInfo {
  public:
    std::vector<Request *> requests;
    std::map<std::string, DimInfo *> dimInfoMap; // AggregatedDimension name is key
    ~FileInfo() {
      for (it_type_diminfo diminfomapiterator = dimInfoMap.begin(); diminfomapiterator != dimInfoMap.end(); diminfomapiterator++) {
        delete diminfomapiterator->second;
      }
      for (size_t j = 0; j < requests.size(); j++) {
        delete requests[j];
      }
    }
  };

  std::map<std::string, FileInfo *> fileInfoMap; // File name is key

  typedef std::map<std::string, FileInfo *>::iterator it_type_file;

  int dimOrdering[CMakeEProfile_MAX_DIMS];

  int *getDimOrder() { return dimOrdering; }

  EProfileUniqueRequests() { readDataAsCDFDouble = false; }
  ~EProfileUniqueRequests() {
    typedef std::map<std::string, FileInfo *>::iterator it_type_file;
    for (it_type_file filemapiterator = fileInfoMap.begin(); filemapiterator != fileInfoMap.end(); filemapiterator++) {

      delete filemapiterator->second;
    }
    //     for(size_t j=0;j<results.size();j++){
    //       delete results[j];
    //     }
    //     results.clear();
  }

  void set(const char *filename, const char *dimName, size_t dimIndex, CT::string dimValue) {

    /* Find the right file based on filename */
    FileInfo *fileInfo = NULL;
    std::map<std::string, FileInfo *>::iterator itf = fileInfoMap.find(filename);
    if (itf != fileInfoMap.end()) {
      fileInfo = (*itf).second;
    } else {
      fileInfo = new FileInfo();
      fileInfoMap.insert(std::pair<std::string, FileInfo *>(filename, fileInfo));
    }

    /* Find the right diminfo based on dimension name */
    DimInfo *dimInfo = NULL;
    std::map<std::string, DimInfo *>::iterator itd = fileInfo->dimInfoMap.find(dimName);
    if (itd != fileInfo->dimInfoMap.end()) {
      dimInfo = (*itd).second;
    } else {
      dimInfo = new DimInfo();
      fileInfo->dimInfoMap.insert(std::pair<std::string, DimInfo *>(dimName, dimInfo));
    }

    dimInfo->dimValuesMap[dimIndex] = dimValue.c_str();

#ifdef CMakeEProfile_DEBUG
//    CDBDebug("Adding %s %d %s",dimName,dimIndex,dimValue.c_str());
#endif
  }

  void addDimSet(DimInfo *dimInfo, int start, std::vector<std::string> valueList) {
#ifdef CMakeEProfile_DEBUG
    CDBDebug("Adding %d with %d values", start, valueList->size());
#endif
    AggregatedDimension *aggregatedValue = new AggregatedDimension();
    aggregatedValue->start = start;
    aggregatedValue->values = valueList;
    dimInfo->aggregatedValues.push_back(aggregatedValue);
  }

  AggregatedDimension *dimensions[CMakeEProfile_MAX_DIMS];

  void nestRequest(it_type_diminfo diminfomapiterator, FileInfo *fileInfo, int depth) {
    if (diminfomapiterator != fileInfo->dimInfoMap.end()) {
      it_type_diminfo currentIt = diminfomapiterator;
      int currentDepth = depth;
      diminfomapiterator++;
      depth++;
      for (size_t j = 0; j < (currentIt->second)->aggregatedValues.size(); j++) {
        AggregatedDimension *aggregatedValue = (currentIt->second)->aggregatedValues[j];
        aggregatedValue->name = (currentIt->first).c_str();
        dimensions[currentDepth] = aggregatedValue;
        nestRequest(diminfomapiterator, fileInfo, depth);
      }
      return;
    } else {
#ifdef CMakeEProfile_DEBUG
      //      CDBDebug("B %d %s",depth,p.c_str());
      CDBDebug("Add request with following:");
#endif
      Request *request = new Request();
      for (int j = 0; j < depth; j++) {
        // CDBDebug("  %d %s %d %d",j,dimensions[j]->name.c_str(),dimensions[j]->start,dimensions[j]->values.size());
        request->dimensions[j] = dimensions[j];
      }
      request->numDims = depth;
      fileInfo->requests.push_back(request);
      return;
    }
  }

  void sortAndAggregate() {
    for (it_type_file filemapiterator = fileInfoMap.begin(); filemapiterator != fileInfoMap.end(); filemapiterator++) {

      for (it_type_diminfo diminfomapiterator = (filemapiterator->second)->dimInfoMap.begin(); diminfomapiterator != (filemapiterator->second)->dimInfoMap.end(); diminfomapiterator++) {
#ifdef CMakeEProfile_DEBUG
        CDBDebug("%s/%s", (filemapiterator->first).c_str(), (diminfomapiterator->first).c_str());
#endif
        map_type_dimvalindex *dimValuesMap = &diminfomapiterator->second->dimValuesMap;
        int currentDimIndex = -1;
        int dimindex;

        int startDimIndex;
        std::vector<std::string> dimValues;
        for (it_type_dimvalindex dimvalindexmapiterator = dimValuesMap->begin(); dimvalindexmapiterator != dimValuesMap->end(); dimvalindexmapiterator++) {
          // const char *filename=(filemapiterator->first).c_str();
          // const char *dimname=(diminfomapiterator->first).c_str();
          dimindex = dimvalindexmapiterator->first;
          const char *dimvalue = dimvalindexmapiterator->second.c_str();

          if (currentDimIndex != -1) {
            if (currentDimIndex == dimindex - 1) {
              currentDimIndex = dimindex;
            } else {

              //*** GO ***
#ifdef CMakeEProfile_DEBUG
              CDBDebug("Print stop at %d", currentDimIndex);
#endif
              currentDimIndex = -1;
              addDimSet(diminfomapiterator->second, startDimIndex, dimValues);
            }
          }

          if (currentDimIndex == -1) {
#ifdef CMakeEProfile_DEBUG
            CDBDebug("Print start at %d", dimindex);
#endif
            currentDimIndex = dimindex;
            startDimIndex = dimindex;
            dimValues.clear();
          }

          if (currentDimIndex != -1) {
#ifdef CMakeEProfile_DEBUG
//              CDBDebug("Add %d / %s",dimindex,dimvalue);
#endif
            dimValues.push_back(dimvalue);
          }
        }
        if (currentDimIndex != -1) {
          //*** GO ***
#ifdef CMakeEProfile_DEBUG
          CDBDebug("Print stop at %d", dimindex);
#endif
          currentDimIndex = -1;
          addDimSet(diminfomapiterator->second, startDimIndex, dimValues);
        }
      }
    }

    // Generate EProfileUniqueRequests
    for (it_type_file filemapiterator = fileInfoMap.begin(); filemapiterator != fileInfoMap.end(); filemapiterator++) {
      nestRequest((filemapiterator->second)->dimInfoMap.begin(), filemapiterator->second, 0);
    }
  }

  void makeRequests(CDrawImage *drawImage, CImageWarper *, CDataSource *dataSource, int, int, CT::string *eProfileJson) {
#ifdef CMakeEProfile_DEBUG
    CDBDebug("\\makeRequests");
#endif
    CDataReader reader;

    reader.open(dataSource, CNETCDFREADER_MODE_OPEN_HEADER);

    int status = 0;
    status = drawImage->createImage(dataSource->srvParams->Geo);

    if (status != 0) {
      CDBError("Unable to create image ");
      return;
    }

    CStyleConfiguration *styleConfiguration = dataSource->getStyle();
    if (styleConfiguration->legendIndex != -1) {
      status = drawImage->createGDPalette(dataSource->srvParams->cfg->Legend[styleConfiguration->legendIndex]);
      if (status != 0) {
        CDBError("Unknown palette type for %s", dataSource->srvParams->cfg->Legend[styleConfiguration->legendIndex]->attr.name.c_str());
        return;
      }
    }
#ifdef CMakeEProfile_DEBUG
    CDBDebug("dataSource->dataObjects.size() = [%d]", dataSource->dataObjects.size());
#endif
    for (size_t dataObjectNr = 0; dataObjectNr < dataSource->dataObjects.size(); dataObjectNr++) {
      CDataSource::DataObject *dataObject = dataSource->getDataObject(dataObjectNr);
      CT::string variableName = dataObject->cdfVariable->name;
      variableName.concat("_backup");
      // Show all requests

      for (it_type_file filemapiterator = fileInfoMap.begin(); filemapiterator != fileInfoMap.end(); filemapiterator++) {
#ifdef CMakeEProfile_DEBUG
        CDBDebug("filemapiterator");
#endif
        //         CT::string ckey;ckey.print("%d%d%s",dX,dY,dataSource->nativeProj4.c_str());
        //         CImageDataWriter::ProjCacheInfo projCacheInfo = CImageDataWriter::GetProjInfo(ckey,drawImage,dataSource,imageWarper, dataSource->srvParams,dX,dY);
        //         CDBDebug("projCacheInfo.isOutsideBBOX == %d",projCacheInfo.isOutsideBBOX);

        // if(projCacheInfo.isOutsideBBOX == false)
        {

          CDFObject *cdfObject = CDFObjectStore::getCDFObjectStore()->getCDFObjectHeader(dataSource, dataSource->srvParams, (filemapiterator->first).c_str());
          CDF::Variable *variable = cdfObject->getVariableNE(variableName.c_str());
          dataObject->cdfVariable = variable;
          if (variable == NULL) {
            CDBError("Variable %s not found", variableName.c_str());
            throw(__LINE__);
          }

          for (size_t j = 0; j < (filemapiterator->second)->requests.size(); j++) {

            Request *request = (filemapiterator->second)->requests[j];
#ifdef CMakeEProfile_DEBUG
            CDBDebug("%s", (filemapiterator->first).c_str());
#endif

            variable->freeData();

            size_t start[variable->dimensionlinks.size() + 2], count[variable->dimensionlinks.size() + 2];
            ptrdiff_t stride[variable->dimensionlinks.size() + 2];

            for (size_t j = 0; j < variable->dimensionlinks.size(); j++) {
              start[j] = 0;
              count[j] = variable->dimensionlinks[j]->getSize();
              stride[j] = 1;
            }
            /*       start[dataSource->dimXIndex] = projCacheInfo.imx;
                   start[dataSource->dimYIndex] = projCacheInfo.imy;
               */

            for (int i = 0; i < request->numDims; i++) {
              CT::string varname = request->dimensions[i]->name;
              int netcdfDimIndex = -1;
              try {

                if (varname.equals("time")) {
                  varname = "time_obs";
                }
                variable->getDimensionIndex(varname.c_str());
              } catch (int e) {
                CDBError("Unable to find dimension [%s]", varname.c_str());
                throw(__LINE__);
              }
              start[netcdfDimIndex] = request->dimensions[i]->start;
              count[netcdfDimIndex] = request->dimensions[i]->values.size();
#ifdef CMakeEProfile_DEBUG
              CDBDebug(">  %d %s %d %d", i, varname.c_str(), request->dimensions[i]->start, request->dimensions[i]->values.size());
#endif
            }

#ifdef CMakeEProfile_DEBUG
            for (size_t i = 0; i < variable->dimensionlinks.size(); i++) {
              CDBDebug("  %d [%d:%d]", i, start[i], count[i]);
            }
#endif

            variable->setType(CDF_FLOAT);
            int status = variable->readData(variable->currentType, start, count, stride, true);

            if (status != 0) {
              CDBError("Unable to read variable %s", variable->name.c_str());
              throw(__LINE__);
            }

            if (status == 0) {
              /**
               * DataPostProc: Here our datapostprocessor comes into action!
               */
              for (size_t dpi = 0; dpi < dataSource->cfgLayer->DataPostProc.size(); dpi++) {
                CServerConfig::XMLE_DataPostProc *proc = dataSource->cfgLayer->DataPostProc[dpi];
                // Algorithm ax+b:
                if (proc->attr.algorithm.equals("ax+b")) {
                  double dfadd_offset = 0;
                  double dfscale_factor = 1;

                  CT::string offsetStr = proc->attr.b.c_str();
                  dfadd_offset = offsetStr.toDouble();
                  CT::string scaleStr = proc->attr.a.c_str();
                  dfscale_factor = scaleStr.toDouble();
                  double *_data = (double *)variable->data;
                  for (size_t j = 0; j < variable->getSize(); j++) {
                    // if(j%10000==0){CDBError("%d = %f",j,_data[j]);}
                    _data[j] = _data[j] * dfscale_factor + dfadd_offset;
                  }
                  // Convert the nodata type
                  dataSource->getDataObject(dataObjectNr)->dfNodataValue = dataSource->getDataObject(dataObjectNr)->dfNodataValue * dfscale_factor + dfadd_offset;
                }
                // Apply units:
                if (proc->attr.units.empty() == false) {
                  dataSource->getDataObject(dataObjectNr)->setUnits(proc->attr.units.c_str());
                }
              }
              /* End of data postproc */
#ifdef CMakeEProfile_DEBUG
              CDBDebug("Read %d elements", variable->getSize());
#endif

              drawEprofile(drawImage, variable, start, count, request, dataSource, eProfileJson);

              //               try{
              //                 expandData(dataObject,variable,start,count,0,request,0);
              //               }catch(int e){
              //                 CDBError("Error in expandData at line %d",e);
              //                 throw(__LINE__);
              //               }
            }
          }
        }
      }

      /*
          try{
            createStructure(dataObject ,drawImage,imageWarper,dataSource,dX,dY,gfiStructure);
          }catch(int e){
            CDBError("Error in createStructure at line %d",e);
            throw(__LINE__);
          }*/
    }
    reader.close();
#ifdef CMakeEProfile_DEBUG
    CDBDebug("/makeRequests");
#endif
  }

  size_t size() { return fileInfoMap.size(); }

  FileInfo *get(size_t index) {
    typedef std::map<std::string, FileInfo *>::iterator it_type_file;
    size_t s = 0;
    for (it_type_file filemapiterator = fileInfoMap.begin(); filemapiterator != fileInfoMap.end(); filemapiterator++) {
      if (s == index) return filemapiterator->second;
      s++;
    }
    return NULL;
  }
};
const char *EProfileUniqueRequests::className = "EProfileUniqueRequests";

int CMakeEProfile::MakeEProfile(CDrawImage *drawImage, CImageWarper *imageWarper, std::vector<CDataSource *> dataSources, int dataSourceIndex, int dX, int dY, CT::string *eProfileJson) {
  CDataSource *dataSource = dataSources[dataSourceIndex];

  EProfileUniqueRequests uniqueRequest;
  /**
   * DataPostProc: Here our datapostprocessor comes into action!
   */
  //   for(size_t dpi=0;dpi<dataSource->cfgLayer->DataPostProc.size();dpi++){
  //     CServerConfig::XMLE_DataPostProc * proc = dataSource->cfgLayer->DataPostProc[dpi];
  //     //Algorithm ax+b:
  //     if(proc->attr.algorithm.equals("ax+b")){
  //       uniqueRequest.readDataAsCDFDouble = true;
  //       break;
  //     }
  //   }

  int numberOfDims = dataSource->requiredDims.size();
  int numberOfSteps = dataSource->getNumTimeSteps();

#ifdef CMakeEProfile_DEBUG
  CDBDebug("1) /*Find all individual files*/");
#endif

  for (int step = 0; step < numberOfSteps; step++) {
    dataSource->setTimeStep(step);
    // CDBDebug("Found file %d %s",step,dataSource->getFileName());
    for (int dimnr = 0; dimnr < numberOfDims; dimnr++) {
      COGCDims *ogcDim = dataSource->requiredDims[dimnr];
      uniqueRequest.set(dataSource->getFileName(), ogcDim->netCDFDimName.c_str(), dataSource->getDimensionIndex(dimnr), dataSource->getDimensionValue(dimnr));
    }
  }

  // Sort
  try {
    uniqueRequest.sortAndAggregate();
  } catch (int e) {
    CDBError("Error in sortAndAggregate at line %d", e);
    throw(__LINE__);
  }

  // Make requests
  try {
    uniqueRequest.makeRequests(drawImage, imageWarper, dataSource, dX, dY, eProfileJson);
  } catch (int e) {
    CDBError("Error in makeRequests at line %d", e);
    throw(__LINE__);
  }

  return 0;
};

int EProfileUniqueRequests::plotHeightRetrieval(CDrawImage *drawImage, CDFObject *cdfObject, const char *varName, CColor c, size_t NrOfDates, double startGraphTime, double startGraphRange,
                                                double graphWidth, double graphHeight, int timeWidth) {

  CT::string newVarName;
  newVarName.print("%s", varName);
  CDF::Variable *plotHeightRetrievalVariable = cdfObject->getVariableNE(newVarName.c_str());
  if (plotHeightRetrievalVariable != NULL) {
    int status = plotHeightRetrievalVariable->readData(CDF_SHORT);
    if (status != 0) {
      CDBError("Unable to read %s data", varName);
      return 1;
    }
  }
  if (plotHeightRetrievalVariable != NULL) {
    double imageWidth = drawImage->Geo->dWidth;
    double imageHeight = drawImage->Geo->dHeight;
    CDF::Variable *varTime = cdfObject->getVariableNE("time_obs");
    if (plotHeightRetrievalVariable->data != NULL) {
      size_t numLayers = 1;

      if (plotHeightRetrievalVariable->dimensionlinks.size() == 2) {
        numLayers = plotHeightRetrievalVariable->dimensionlinks[1]->getSize();
      }

      size_t numLoaded = plotHeightRetrievalVariable->dimensionlinks[0]->getSize();
      if (NrOfDates > numLoaded - 1) NrOfDates = numLoaded - 1;

      for (size_t time = 0; time < NrOfDates; time++) {
        for (size_t layer = 0; layer < numLayers; layer++) {
          int x = int(((((double *)varTime->data)[time] - startGraphTime) / graphWidth) * imageWidth);
          float h = ((short *)plotHeightRetrievalVariable->data)[time * numLayers + layer];
          if (h > 0) {
            int y = imageHeight - int(((h - startGraphRange) / graphHeight) * imageHeight);
            drawImage->line(x - 1 + timeWidth / 2, y - 1, x + 1 + timeWidth / 2, y + 1, c);
            drawImage->line(x - 1 + timeWidth / 2, y + 1, x + 1 + timeWidth / 2, y - 1, c);
          }
        }
      }
    }
  }
  return 0;
}

int EProfileUniqueRequests::drawEprofile(CDrawImage *drawImage, CDF::Variable *variable, size_t *, size_t *count, EProfileUniqueRequests::Request *, CDataSource *dataSource,
                                         CT::string *eProfileJson) {

  // CTime adagucTime;
  // adagucTime->init(((CDFObject*)variable->getParentCDFObject())->getVariableNE("time_obs"));
  CTime *adagucTime = CTime::GetCTimeInstance(((CDFObject *)variable->getParentCDFObject())->getVariableNE("time_obs"));
  if (adagucTime == nullptr) {
    CDBDebug(CTIME_GETINSTANCE_ERROR_MESSAGE);
    return 1;
  }

  COGCDims *ogcDim = dataSource->requiredDims[0];

#ifdef CMakeEProfile_DEBUG
  CDBDebug("count %d", count[0]);
  ;
  CDBDebug("total %d", ogcDim->uniqueValues.size());
  CDBDebug("ogcDim->uniqueValues[0].c_str()) = %s", ogcDim->uniqueValues[0].c_str());
#endif

  CT::string rangeVarName = variable->dimensionlinks[1]->name.c_str();
#ifdef CMakeEProfile_DEBUG
  CDBDebug("Reading range var with name %s", rangeVarName.c_str());
#endif
  CDF::Variable *varRange = ((CDFObject *)variable->getParentCDFObject())->getVariableNE(rangeVarName.c_str());
  if (varRange == NULL) {
    CDBError("%s not found", rangeVarName.c_str());
    return -1;
  }
  varRange->readData(CDF_FLOAT);

  CDF::Variable *varTime = ((CDFObject *)variable->getParentCDFObject())->getVariableNE("time_obs");
  if (varTime == NULL) {
    CDBError("%s not found", rangeVarName.c_str());
    return -1;
  }
  varTime->readData(CDF_DOUBLE);
  if (varTime->getSize() != count[0]) {
    CDBError("varTime->getSize()!=count[0] : %d!=%d", varTime->getSize(), count[0]);
    return 1;
  }

  double startGraphTime = adagucTime->dateToOffset(adagucTime->freeDateStringToDate(ogcDim->uniqueValues[0].c_str()));
  double stopGraphTime = adagucTime->dateToOffset(adagucTime->freeDateStringToDate(ogcDim->uniqueValues[ogcDim->uniqueValues.size() - 1].c_str()));

  double startGraphRange = ((float *)varRange->data)[0];
  double stopGraphRange = ((float *)varRange->data)[variable->dimensionlinks[1]->getSize() - 1];

  if (variable->dimensionlinks[1]->getSize() != count[1]) {
    CDBError("Range not equal");
    return 1;
  }

  int foundTimeDim = -1;
  for (size_t k = 0; k < dataSource->srvParams->requestDims.size(); k++) {
    if (dataSource->srvParams->requestDims[k]->name.equals("time")) {
      foundTimeDim = k;
      break;
    }
  }

  if (foundTimeDim != -1) {
    CT::string *timeEntries = dataSource->srvParams->requestDims[foundTimeDim]->value.splitToArray("/");
    if (timeEntries->count == 2) {
#ifdef CMakeEProfile_DEBUG
      CDBDebug("time=%s", dataSource->srvParams->requestDims[foundTimeDim]->value.c_str());
#endif
      startGraphTime = adagucTime->dateToOffset(adagucTime->freeDateStringToDate(timeEntries[0].c_str()));
      stopGraphTime = adagucTime->dateToOffset(adagucTime->freeDateStringToDate(timeEntries[1].c_str()));
    }
    delete[] timeEntries;
  }

  int foundElevationDim = -1;
  for (size_t k = 0; k < dataSource->srvParams->requestDims.size(); k++) {
    if (dataSource->srvParams->requestDims[k]->name.equals("elevation")) {
      foundElevationDim = k;
      break;
    }
  }

  if (foundElevationDim != -1) {
    CT::string *elevationEntries = dataSource->srvParams->requestDims[foundElevationDim]->value.splitToArray("/");
    if (elevationEntries->count == 2) {
#ifdef CMakeEProfile_DEBUG
      CDBDebug("elevation=%s", dataSource->srvParams->requestDims[foundElevationDim]->value.c_str());
#endif
      startGraphRange = elevationEntries[0].toDouble();
      ;
      stopGraphRange = elevationEntries[1].toDouble();
      ;
    }
    delete[] elevationEntries;
  }

  if (dataSource->srvParams->InfoFormat.equals("application/json")) {
    float *data = (float *)varRange->data;
    eProfileJson->concat("{");
    CT::string units = dataSource->getDataObject(0)->getUnits();
    if (!units.empty()) {
      eProfileJson->printconcat("\"units\":\"%s\",", encodeJSON(units).c_str());
    } else {
      eProfileJson->printconcat("\"units\":null,");
    }
    CDF::Attribute *unitsY = varRange->getAttributeNE("units");
    CDF::Attribute *standardName = variable->getAttributeNE("standard_name");
    CDF::Attribute *longName = variable->getAttributeNE("long_name");
    CT::string layerName = dataSource->getLayerName();
    CT::string layerTitle = dataSource->getLayerTitle();

    std::string dq = "\"";

    eProfileJson->printconcat("\"units_y\":%s,", (unitsY != NULL ? dq + std::string(unitsY->getDataAsString().c_str()) + dq : "null").c_str());
    eProfileJson->printconcat("\"standard_name\":%s,", (standardName != NULL ? dq + std::string(standardName->getDataAsString().c_str()) + dq : "null").c_str());
    eProfileJson->printconcat("\"long_name\":%s,", (longName != NULL ? dq + encodeJSON(longName->getDataAsString()) + dq : "null").c_str());
    eProfileJson->printconcat("\"layer_name\":%s,", (layerName.empty() == false ? dq + encodeJSON(layerName) + dq : "null").c_str());
    eProfileJson->printconcat("\"layer_title\":%s,", (layerTitle.empty() == false ? dq + encodeJSON(layerTitle) + dq : "null").c_str());
    eProfileJson->printconcat("\"numValues\":%d,", varRange->getSize());
    eProfileJson->printconcat("\"name\":\"%s\",", encodeJSON(variable->name.replace("_backup", "")).c_str());

    CDBDebug("%d", variable->getSize());

    size_t colOffset = varRange->getSize() * 0;
    if (count[0] > 1) {
      for (size_t timeIndex = 0; timeIndex < count[0] - 1; timeIndex++) {
        double dataTimeStart = ((double *)varTime->data)[timeIndex];
        double dataTimeEnd = ((double *)varTime->data)[timeIndex + 1];
        if (startGraphTime > dataTimeStart && startGraphTime < dataTimeEnd) {

          colOffset = timeIndex * varRange->getSize();
        }
      }
    }

    CDBDebug("Querying for time index %d and file %s", colOffset, dataSource->getFileName());

    // Make profile object
    eProfileJson->concat("\"profile\":{");
    // Make height object
    eProfileJson->concat("\n\"heights\":[");
    CDBDebug("startGraphRange %f %f", startGraphRange, stopGraphRange);
    bool firstElDone = false;
    for (size_t j = 0; j < varRange->getSize(); j += 1) {
      float v = float(data[j]);
      if (v >= startGraphRange && v < stopGraphRange) {
        if (firstElDone) {
          eProfileJson->concat(",");
        };
        firstElDone = true;
        if (v == v) {
          eProfileJson->printconcat("%g", v);
        } else {
          eProfileJson->printconcat("null");
        }
      }
    }
    eProfileJson->concat("],");
    // Make values object
    eProfileJson->concat("\n\"values\":[");
    firstElDone = false;
    for (size_t j = 0; j < varRange->getSize(); j += 1) {
      float v = float(data[j]);
      if (v >= startGraphRange && v < stopGraphRange) {
        if (firstElDone) {
          eProfileJson->concat(",");
        };
        firstElDone = true;
        if (variable->getType() == CDF_FLOAT) {
          float v = ((float *)variable->data)[j + colOffset];
          if (v == v) {
            eProfileJson->printconcat("%g", v);
          } else {
            eProfileJson->printconcat("null");
          }
        }
        if (variable->getType() == CDF_DOUBLE) {
          double v = ((double *)variable->data)[j + colOffset];
          if (v == v) {
            eProfileJson->printconcat("%g", v);
          } else {
            eProfileJson->printconcat("null");
          }
        }
      }
    }
    eProfileJson->concat("]");

    eProfileJson->concat("\n}}");

    return 0;
  }

  double graphWidth = stopGraphTime - startGraphTime;
  double graphHeight = stopGraphRange - startGraphRange;

  double imageWidth = drawImage->Geo->dWidth;
  double imageHeight = drawImage->Geo->dHeight;

  if (graphWidth <= 0) {
    graphWidth = imageWidth;
  }
  if (graphHeight <= 0) {
    graphHeight = imageHeight;
  }

#ifdef CMakeEProfile_DEBUG
  CDBDebug("startGraphTime = %f stopGraphTime = %f graphWidth = %f imageWidth = %f", startGraphTime, stopGraphTime, graphWidth, imageWidth);
  CDBDebug("startGraphRange = %f stopGraphTime = %f graphWidth = %f imageWidth = %f", startGraphRange, stopGraphTime, graphHeight, imageHeight);
#endif

#ifdef CMakeEProfile_DEBUG
  CDBDebug("Number of timesteps: %d", count[0]);
#endif

  CStyleConfiguration *styleConfiguration = dataSource->getStyle();

  double dfNodataValue = dataSource->getDataObject(0)->dfNodataValue;
  float nodataValue = (float)dfNodataValue;
  float legendValueRange = styleConfiguration->hasLegendValueRange;
  float legendLowerRange = styleConfiguration->legendLowerRange;
  float legendUpperRange = styleConfiguration->legendUpperRange;
  bool hasNodataValue = dataSource->getDataObject(0)->hasNodataValue;
  float legendLogAsLog = 0;
  float legendLog = styleConfiguration->legendLog;
  if (legendLog > 0) {
    legendLogAsLog = log10(legendLog);
  }
  float legendScale = styleConfiguration->legendScale;
  float legendOffset = styleConfiguration->legendOffset;

  std::vector<CMakeEProfile::DayPass> dayPasses;
  int minWidth = 0;

  // Fallback validity length of the observation, in case there is only one observation in the file
  double duration = DEFAULT_VALIDITY_LENGTH_OF_OBSERVATION_IN_SECONDS;

  CDF::Attribute *durationAttribute = varTime->getAttributeNE("duration");
  if (durationAttribute != NULL && durationAttribute->getType() == CDF_DOUBLE) {
    durationAttribute->getData(&duration, 1);
  }

  for (size_t time = 0; time < count[0]; time++) {

    int x1 = int(((((double *)varTime->data)[time] - startGraphTime) / graphWidth) * imageWidth);
    int x2 = 0;

    if (time < count[0] - 1) {
      x2 = int(((((double *)varTime->data)[time + 1] - startGraphTime) / graphWidth) * imageWidth);
      if (minWidth == 0) {
        minWidth = x2 - x1;
      } else {
        if (x2 - x1 < minWidth) {
          minWidth = x2 - x1;
        }
      }
    } else {
      x2 = x1 + int(((duration / graphWidth) * imageWidth) + 0.5) + 1;
    }
    if (x2 >= 0 && x1 < imageWidth && x1 < x2) {

      // CDBDebug("x1 = %d fileTime = %f",x1,fileTime);
      for (size_t range = 0; range < count[1] - 1; range++) {

        int y1 = imageHeight - int(((((float *)varRange->data)[range + 1] - startGraphRange) / graphHeight) * imageHeight);
        int y2 = imageHeight - int(((((float *)varRange->data)[range] - startGraphRange) / graphHeight) * imageHeight);
        if (y2 >= 0 && y1 < imageHeight && y1 < y2) {
          float *data = (float *)(variable->data);
          size_t p = range + time * count[1];
          float val = data[p];

          bool isNodata = false;
          if (hasNodataValue) {
            if (val == nodataValue) isNodata = true;
          }
          if (!(val == val)) isNodata = true;
          if (!isNodata)
            if (legendValueRange)
              if (val < legendLowerRange || val > legendUpperRange) isNodata = true;
          if (!isNodata) {
            if (legendLog != 0) {
              if (val > 0) {
                val = (log10(val) / legendLogAsLog);
              } else
                val = (-legendOffset);
            }

            int pcolorind = (int)(val * legendScale + legendOffset);
            // val+=legendOffset;
            if (pcolorind >= 239)
              pcolorind = 239;
            else if (pcolorind <= 0)
              pcolorind = 0;

            for (int y = y1; y < y2; y++) {
              for (int x = x1; x < x2; x++) {
                drawImage->setPixelIndexed(x, y, pcolorind);
              }
            }
          }
        }
      }
    }

    dayPasses.push_back(CMakeEProfile::DayPass(x1, ((double *)varTime->data)[time]));
  }

  /*
   CT::string dateStr  =adagucTime->dateToISOString(adagucTime->offsetToDate(((double*)varTime->data)[time]));
       drawImage->setText(dateStr.c_str(),dateStr.length(),x1,1,CColor(0,0,0,0),12);*/

  plotHeightRetrieval(drawImage, ((CDFObject *)variable->getParentCDFObject()), "cbh", CColor(0, 0, 255, 255), count[0], startGraphTime, startGraphRange, graphWidth, graphHeight, minWidth);
  /*  plotHeightRetrieval(drawImage,((CDFObject*)variable->getParentCDFObject()),"pbl",CColor(0,0,255,255),count[0],startGraphTime,startGraphRange,graphWidth,graphHeight,minWidth);
     plotHeightRetrieval(drawImage,((CDFObject*)variable->getParentCDFObject()),"vor",CColor(255,255,255,255),count[0],startGraphTime,startGraphRange,graphWidth,graphHeight,minWidth);
   */  //plotHeightRetrieval(drawImage,((CDFObject*)variable->getParentCDFObject()),"cdp",CColor(0,0,0,255),count[0],startGraphTime,startGraphRange,graphWidth,graphHeight);
  //

  for (size_t j = 0; j < dayPasses.size(); j++) {
    CTime::Date d = adagucTime->offsetToDate(dayPasses[j].offset);
    if (d.minute == 0 && d.hour == 0) {
      CT::string dateStr = adagucTime->dateToISOString(d);
      dateStr.setSize(10);
      drawImage->setText(dateStr.c_str(), dateStr.length(), dayPasses[j].x + 4, 5, CColor(0, 0, 0, 0), 12);

      for (int y = 0; y < imageHeight; y++) {
        drawImage->setPixelTrueColor(dayPasses[j].x, y, 0, 0, 255, 255);
      }
    }
  }

  // drawImage->line(0,0,100,100,248);
  return 0;
}
