import { Empty, FileInfoRequest, FileInfoResponse, FileServiceClient, StatusResponse } from "../proto/fitsReaderProto/file_service";
import { credentials } from "@grpc/grpc-js";
import { promisify } from "util";

export class FitsWorker {
  readonly checkStatus: (request: Empty) => Promise<StatusResponse>;
  readonly getFileInfo: (request: FileInfoRequest) => Promise<FileInfoResponse>;

  private _connected = false;

  get connected() {
    return this._connected;
  }

  constructor(port: number = 8080) {
    const WORKER_URL = `0.0.0.0:${port}`;
    const client = new FileServiceClient(WORKER_URL, credentials.createInsecure());
    this.checkStatus = promisify<Empty, StatusResponse>(client.checkStatus).bind(client);
    this.getFileInfo = promisify<FileInfoRequest, FileInfoResponse>(client.getFileInfo).bind(client);
    client.waitForReady(Date.now() + 1000, (err) => {
      if (err) {
        console.error(err);
        this._connected = false;
      } else {
        console.log(`worker connected to port ${port}`);
        this._connected = true;
      }
    });
  }
}
