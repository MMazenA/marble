import * as jspb from 'google-protobuf'



export class AggregatesRequest extends jspb.Message {
  getTicker(): string;
  setTicker(value: string): AggregatesRequest;

  getFromDate(): string;
  setFromDate(value: string): AggregatesRequest;

  getToDate(): string;
  setToDate(value: string): AggregatesRequest;

  getTimeSpan(): timespan_options;
  setTimeSpan(value: timespan_options): AggregatesRequest;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): AggregatesRequest.AsObject;
  static toObject(includeInstance: boolean, msg: AggregatesRequest): AggregatesRequest.AsObject;
  static serializeBinaryToWriter(message: AggregatesRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): AggregatesRequest;
  static deserializeBinaryFromReader(message: AggregatesRequest, reader: jspb.BinaryReader): AggregatesRequest;
}

export namespace AggregatesRequest {
  export type AsObject = {
    ticker: string;
    fromDate: string;
    toDate: string;
    timeSpan: timespan_options;
  };
}

export class AggregatesResponse extends jspb.Message {
  getTicker(): string;
  setTicker(value: string): AggregatesResponse;

  getQueryCount(): number;
  setQueryCount(value: number): AggregatesResponse;

  getRequestId(): string;
  setRequestId(value: string): AggregatesResponse;

  getResultsCount(): number;
  setResultsCount(value: number): AggregatesResponse;

  getCount(): number;
  setCount(value: number): AggregatesResponse;

  getStatus(): string;
  setStatus(value: string): AggregatesResponse;

  getAggregateBarsList(): Array<AggregateBar>;
  setAggregateBarsList(value: Array<AggregateBar>): AggregatesResponse;
  clearAggregateBarsList(): AggregatesResponse;
  addAggregateBars(value?: AggregateBar, index?: number): AggregateBar;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): AggregatesResponse.AsObject;
  static toObject(includeInstance: boolean, msg: AggregatesResponse): AggregatesResponse.AsObject;
  static serializeBinaryToWriter(message: AggregatesResponse, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): AggregatesResponse;
  static deserializeBinaryFromReader(message: AggregatesResponse, reader: jspb.BinaryReader): AggregatesResponse;
}

export namespace AggregatesResponse {
  export type AsObject = {
    ticker: string;
    queryCount: number;
    requestId: string;
    resultsCount: number;
    count: number;
    status: string;
    aggregateBarsList: Array<AggregateBar.AsObject>;
  };
}

export class AggregateBar extends jspb.Message {
  getOpen(): number;
  setOpen(value: number): AggregateBar;

  getClose(): number;
  setClose(value: number): AggregateBar;

  getHigh(): number;
  setHigh(value: number): AggregateBar;

  getLow(): number;
  setLow(value: number): AggregateBar;

  getN(): number;
  setN(value: number): AggregateBar;

  getOtc(): boolean;
  setOtc(value: boolean): AggregateBar;

  getT(): number;
  setT(value: number): AggregateBar;

  getVolume(): number;
  setVolume(value: number): AggregateBar;

  getVolumeWeighted(): number;
  setVolumeWeighted(value: number): AggregateBar;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): AggregateBar.AsObject;
  static toObject(includeInstance: boolean, msg: AggregateBar): AggregateBar.AsObject;
  static serializeBinaryToWriter(message: AggregateBar, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): AggregateBar;
  static deserializeBinaryFromReader(message: AggregateBar, reader: jspb.BinaryReader): AggregateBar;
}

export namespace AggregateBar {
  export type AsObject = {
    open: number;
    close: number;
    high: number;
    low: number;
    n: number;
    otc: boolean;
    t: number;
    volume: number;
    volumeWeighted: number;
  };
}

export class AggregatesStreamRequest extends jspb.Message {
  getFilter(): string;
  setFilter(value: string): AggregatesStreamRequest;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): AggregatesStreamRequest.AsObject;
  static toObject(includeInstance: boolean, msg: AggregatesStreamRequest): AggregatesStreamRequest.AsObject;
  static serializeBinaryToWriter(message: AggregatesStreamRequest, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): AggregatesStreamRequest;
  static deserializeBinaryFromReader(message: AggregatesStreamRequest, reader: jspb.BinaryReader): AggregatesStreamRequest;
}

export namespace AggregatesStreamRequest {
  export type AsObject = {
    filter: string;
  };
}

export class AggregatesUpdate extends jspb.Message {
  getId(): number;
  setId(value: number): AggregatesUpdate;

  getName(): string;
  setName(value: string): AggregatesUpdate;

  getNote(): string;
  setNote(value: string): AggregatesUpdate;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): AggregatesUpdate.AsObject;
  static toObject(includeInstance: boolean, msg: AggregatesUpdate): AggregatesUpdate.AsObject;
  static serializeBinaryToWriter(message: AggregatesUpdate, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): AggregatesUpdate;
  static deserializeBinaryFromReader(message: AggregatesUpdate, reader: jspb.BinaryReader): AggregatesUpdate;
}

export namespace AggregatesUpdate {
  export type AsObject = {
    id: number;
    name: string;
    note: string;
  };
}

export class StreamMessage extends jspb.Message {
  getId(): number;
  setId(value: number): StreamMessage;

  getPayload(): string;
  setPayload(value: string): StreamMessage;

  serializeBinary(): Uint8Array;
  toObject(includeInstance?: boolean): StreamMessage.AsObject;
  static toObject(includeInstance: boolean, msg: StreamMessage): StreamMessage.AsObject;
  static serializeBinaryToWriter(message: StreamMessage, writer: jspb.BinaryWriter): void;
  static deserializeBinary(bytes: Uint8Array): StreamMessage;
  static deserializeBinaryFromReader(message: StreamMessage, reader: jspb.BinaryReader): StreamMessage;
}

export namespace StreamMessage {
  export type AsObject = {
    id: number;
    payload: string;
  };
}

export enum timespan_options {
  SECOND = 0,
  MINUTE = 1,
  HOUR = 2,
  DAY = 3,
  WEEK = 4,
  MONTH = 5,
  QUARTER = 6,
  YEAR = 7,
}
