import {
  Empty, FileCloseRequest,
  FileInfoRequest,
  FileInfoResponse, FileOpenRequest,
  FileServiceClient,
  ImageDataRequest,
  ImageDataResponse,
  SpectralProfileRequest, SpectralProfileResponse,
  StatusResponse,
} from "../proto/fitsReaderProto/file_service";
import { credentials } from "@grpc/grpc-js";
import { promisify } from "util";

export class FitsWorker {
  readonly checkStatus: (request: Empty) => Promise<StatusResponse>;
  readonly getFileInfo: (request: FileInfoRequest) => Promise<FileInfoResponse>;
  readonly getImageData: (request: ImageDataRequest) => Promise<ImageDataResponse>;
  readonly getSpectralProfile: (request: SpectralProfileRequest) => Promise<SpectralProfileResponse>;
  readonly openFile: (request: FileOpenRequest) => Promise<StatusResponse>;
  readonly closeFile: (request: FileCloseRequest) => Promise<StatusResponse>;

  private _connected = false;
  private _readyResolves: (() => void)[] = [];
  private _rejectResolves: ((err: Error) => void)[] = [];

  get connected() {
    return this._connected;
  }

  ready() {
    return new Promise<void>((resolve, reject) => {
      if (this._connected) {
        resolve();
        return;
      }

      this._readyResolves.push(resolve);
      this._rejectResolves.push(reject);
    });
  }

  constructor(port: number = 8080) {
    const WORKER_URL = `0.0.0.0:${port}`;
    const client = new FileServiceClient(WORKER_URL, credentials.createInsecure());

    this.checkStatus = promisify<Empty, StatusResponse>(client.checkStatus).bind(client);
    this.getFileInfo = promisify<FileInfoRequest, FileInfoResponse>(client.getFileInfo).bind(client);
    this.getImageData = promisify<ImageDataRequest, ImageDataResponse>(client.getImageData).bind(client);
    this.getSpectralProfile = promisify<SpectralProfileRequest, SpectralProfileResponse>(client.getSpectralProfile).bind(client);
    this.openFile = promisify<FileOpenRequest, StatusResponse>(client.openFile).bind(client);
    this.closeFile = promisify<FileCloseRequest, StatusResponse>(client.closeFile).bind(client);

    client.waitForReady(Date.now() + 1000, (err) => {
      if (err) {
        console.error(err);
        this._connected = false;
        for (const reject of this._rejectResolves) {
          reject(err);
        }
      } else {
        this._connected = true;
        for (const resolve of this._readyResolves) {
          resolve();
        }
      }
    });
  }
}
