# generated by datamodel-codegen:
#   filename:  ogcapi-features-1.yaml
#   timestamp: 2023-10-11T09:15:03+00:00

from __future__ import annotations

from datetime import datetime
from enum import Enum
from typing import Any, Dict, List, Optional, Union

from pydantic import BaseModel, Field, RootModel, conint


class ConfClasses(BaseModel):
    conformsTo: List[str]


class Exception(BaseModel):
    code: str
    description: Optional[str] = None


class Crs(Enum):
    http___www_opengis_net_def_crs_OGC_1_3_CRS84 = (
        'http://www.opengis.net/def/crs/OGC/1.3/CRS84'
    )


class Spatial(BaseModel):
    bbox: Optional[List[List[float]]] = Field(
        None,
        description='One or more bounding boxes that describe the spatial extent of the dataset.\nIn the Core only a single bounding box is supported. Extensions may support\nadditional areas. If multiple areas are provided, the union of the bounding\nboxes describes the spatial extent.',
        min_length=1,
    )
    crs: Optional[Crs] = Field(
        'http://www.opengis.net/def/crs/OGC/1.3/CRS84',
        description='Coordinate reference system of the coordinates in the spatial extent\n(property `bbox`). The default reference system is WGS 84 longitude/latitude.\nIn the Core this is the only supported coordinate reference system.\nExtensions may support additional coordinate reference systems and add\nadditional enum values.',
    )


class IntervalItem(RootModel[List[Any]]):
    root: List[Any] = Field(
        ...,
        description='Begin and end times of the time interval. The timestamps are in the\ntemporal coordinate reference system specified in `trs`. By default\nthis is the Gregorian calendar.',
        examples=[['2011-11-11T12:22:11Z', None]],
    )


class Trs(Enum):
    http___www_opengis_net_def_uom_ISO_8601_0_Gregorian = (
        'http://www.opengis.net/def/uom/ISO-8601/0/Gregorian'
    )


class Temporal(BaseModel):
    interval: Optional[List[IntervalItem]] = Field(
        None,
        description='One or more time intervals that describe the temporal extent of the dataset.\nThe value `null` is supported and indicates an unbounded interval end.\nIn the Core only a single time interval is supported. Extensions may support\nmultiple intervals. If multiple intervals are provided, the union of the\nintervals describes the temporal extent.',
        min_length=1,
    )
    trs: Optional[Trs] = Field(
        'http://www.opengis.net/def/uom/ISO-8601/0/Gregorian',
        description='Coordinate reference system of the coordinates in the temporal extent\n(property `interval`). The default reference system is the Gregorian calendar.\nIn the Core this is the only supported temporal coordinate reference system.\nExtensions may support additional temporal coordinate reference systems and add\nadditional enum values.',
    )


class Extent(BaseModel):
    spatial: Optional[Spatial] = Field(
        None, description='The spatial extent of the features in the collection.'
    )
    temporal: Optional[Temporal] = Field(
        None, description='The temporal extent of the features in the collection.'
    )


class Type(Enum):
    FeatureCollection = 'FeatureCollection'


class Type1(Enum):
    Feature = 'Feature'


class Type2(Enum):
    GeometryCollection = 'GeometryCollection'


class Type3(Enum):
    LineString = 'LineString'


class Coordinate(RootModel[List[Any]]):
    root: List[Any]


class LinestringGeoJSON(BaseModel):
    type: Type3
    coordinates: List[Coordinate] = Field(..., min_length=2)


class Link(BaseModel):
    href: str = Field(..., examples=['http://data.example.com/buildings/123'])
    rel: Optional[str] = Field(None, examples=['alternate'])
    type: Optional[str] = Field(None, examples=['application/geo+json'])
    hreflang: Optional[str] = Field(None, examples=['en'])
    title: Optional[str] = Field(None, examples=['Trierer Strasse 70, 53115 Bonn'])
    length: Optional[int] = None


class Type4(Enum):
    MultiLineString = 'MultiLineString'


class MultilinestringGeoJSON(BaseModel):
    type: Type4
    coordinates: List[List[Coordinate]]


class Type5(Enum):
    MultiPoint = 'MultiPoint'


class MultipointGeoJSON(BaseModel):
    type: Type5
    coordinates: List[List[float]]


class Type6(Enum):
    MultiPolygon = 'MultiPolygon'


class MultipolygonGeoJSON(BaseModel):
    type: Type6
    coordinates: List[List[List[Coordinate]]]


class NumberMatched(RootModel[conint(ge=0)]):
    root: conint(ge=0) = Field(
        ...,
        description='The number of features of the feature type that match the selection\nparameters like `bbox`.',
        examples=[127],
    )


class NumberReturned(RootModel[conint(ge=0)]):
    root: conint(ge=0) = Field(
        ...,
        description='The number of features in the feature collection.\n\nA server may omit this information in a response, if the information\nabout the number of features is not known or difficult to compute.\n\nIf the value is provided, the value shall be identical to the number\nof items in the "features" array.',
        examples=[10],
    )


class Type7(Enum):
    Point = 'Point'


class PointGeoJSON(BaseModel):
    type: Type7
    coordinates: List[float] = Field(..., min_length=2)


class Type8(Enum):
    Polygon = 'Polygon'


class PolygonGeoJSON(BaseModel):
    type: Type8
    coordinates: List[List[Coordinate]]


class TimeStamp(RootModel[datetime]):
    root: datetime = Field(
        ...,
        description='This property indicates the time and date when the response was generated.',
        examples=['2017-08-17T08:05:32Z'],
    )


class Collection(BaseModel):
    id: str = Field(
        ...,
        description='identifier of the collection used, for example, in URIs',
        examples=['address'],
    )
    title: Optional[str] = Field(
        None, description='human readable title of the collection', examples=['address']
    )
    description: Optional[str] = Field(
        None,
        description='a description of the features in the collection',
        examples=['An address.'],
    )
    links: List[Link] = Field(
        ...,
        examples=[
            {'href': 'http://data.example.com/buildings', 'rel': 'item'},
            {
                'href': 'http://example.com/concepts/buildings.html',
                'rel': 'describedby',
                'type': 'text/html',
            },
        ],
    )
    extent: Optional[Extent] = None
    itemType: Optional[str] = Field(
        'feature',
        description="indicator about the type of the items in the collection (the default value is 'feature').",
    )
    crs: Optional[List[str]] = Field(
        ['http://www.opengis.net/def/crs/OGC/1.3/CRS84'],
        description='the list of coordinate reference systems supported by the service',
        examples=[[
            'http://www.opengis.net/def/crs/OGC/1.3/CRS84',
            'http://www.opengis.net/def/crs/EPSG/0/4326',
        ]],
    )


class Collections(BaseModel):
    links: List[Link]
    collections: List[Collection]


class LandingPage(BaseModel):
    title: Optional[str] = Field(None, examples=['Buildings in Bonn'])
    description: Optional[str] = Field(
        None,
        examples=['Access to data about buildings in the city of Bonn via a Web API that conforms to the OGC API Features specification.'],
    )
    links: List[Link]


class FeatureCollectionGeoJSON(BaseModel):
    type: Type
    features: List[FeatureGeoJSON]
    links: Optional[List[Link]] = None
    timeStamp: Optional[TimeStamp] = None
    numberMatched: Optional[NumberMatched] = None
    numberReturned: Optional[NumberReturned] = None


class FeatureGeoJSON(BaseModel):
    type: Type1
    geometry: GeometryGeoJSON
    properties: Dict[str, Any]
    id: Optional[Union[str, int]] = None
    links: Optional[List[Link]] = None


class GeometrycollectionGeoJSON(BaseModel):
    type: Type2
    geometries: List[GeometryGeoJSON]


class GeometryGeoJSON(
    RootModel[
        Union[
            PointGeoJSON,
            MultipointGeoJSON,
            LinestringGeoJSON,
            MultilinestringGeoJSON,
            PolygonGeoJSON,
            MultipolygonGeoJSON,
            GeometrycollectionGeoJSON,
        ]
    ]
):
    root: Union[
        PointGeoJSON,
        MultipointGeoJSON,
        LinestringGeoJSON,
        MultilinestringGeoJSON,
        PolygonGeoJSON,
        MultipolygonGeoJSON,
        GeometrycollectionGeoJSON,
    ]



FeatureCollectionGeoJSON.model_rebuild()
FeatureGeoJSON.model_rebuild()
GeometryGeoJSON.model_rebuild()