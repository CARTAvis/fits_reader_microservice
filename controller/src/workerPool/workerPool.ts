import { v4 as uuidv4 } from "uuid";

import { FitsWorker } from "./fitsWorker";
import { SpectralProfileResponse } from "../proto/fitsReaderProto/file_service";
import { bytesToFloat32 } from "../utils/arrays";

export class WorkerPool {
  readonly workers: FitsWorker[];
  readonly useBuffering: boolean;

  ready() {
    return Promise.all(this.workers.map((worker) => worker.ready()));
  }

  constructor(workerCount = 4, startPort = 8080, buffer = false) {
    if (workerCount < 1) {
      throw new Error("Worker count must be at least 1");
    }

    this.useBuffering = buffer;
    this.workers = [];
    for (let i = 0; i < workerCount; i++) {
      this.workers.push(new FitsWorker(startPort + i));
    }
  }

  get connectedWorkers() {
    return this.workers.filter((worker) => worker.connected);
  }

  get allConnected() {
    return this.workers.every(w => w.connected);
  }

  get primaryWorker() {
    return this.workers[0];
  }

  get firstConnectedWorker() {
    return this.workers.find((worker) => worker.connected);
  }

  get randomConnectedWorker() {
    return this.connectedWorkers?.[Math.floor(Math.random() * this.connectedWorkers.length)];
  }

  async checkStatus() {
    return this.primaryWorker.checkStatus({});
  }

  async openFile(fileName: string, hdu: string = "") {
    const uuid = uuidv4();
    const promises = this.workers.map((worker) => worker.openFile({ fileName, hdu, uuid }));
    return Promise.all(promises).then(responses => {
      return responses.every(res => res.status) ? { uuid } : undefined;
    });
  }

  async closeFile(uuid: string) {
    const promises = this.workers.map((worker) => worker.closeFile({ uuid }));
    return Promise.all(promises).then(responses => {
      return responses.every(res => res.status);
    });
  }

  async getFileInfo(uuid: string) {
    return this.primaryWorker?.getFileInfo({ uuid });
  }

  async getImageData(uuid: string, start: number[], numPixels: number = 1, workerIndex?: number) {
    const worker = workerIndex !== undefined ? this.workers[workerIndex] : this.randomConnectedWorker;
    return worker?.getImageData({ uuid, start, numPixels });
  }

  async getSpectralProfile(uuid: string, x: number, y: number, z: number, numPixels: number, width = 1, height = 1, numWorkers?: number) {
    if (!numWorkers) {
      numWorkers = this.workers.length;
    }
    const pixelsPerWorker = Math.floor(numPixels / numWorkers);
    const promises = new Array<Promise<SpectralProfileResponse>>();
    for (let i = 0; i < numWorkers; i++) {
      const zStart = z + i * pixelsPerWorker;
      // Last worker gets the remainder
      const numPixelsInChunk = (i === numWorkers - 1) ? numPixels - i * pixelsPerWorker : pixelsPerWorker;
      const worker = this.workers[i % this.workers.length];
      promises.push(worker.getSpectralProfile({ uuid, x, y, z: zStart, width, height, numPixels: numPixelsInChunk, buffered: this.useBuffering }));
    }

    return Promise.all(promises).then(res => {
      const data = new Float32Array(numPixels);
      let offset = 0;
      for (const response of res) {
        const chunk = bytesToFloat32(response.data);
        data.set(chunk, offset);
        offset += chunk.length;
      }
      return { data };
    });
  }
}
