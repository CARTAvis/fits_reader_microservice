import { FitsWorker } from "./fitsWorker";

export class WorkerPool {
  readonly workers: FitsWorker[];

  constructor(workerCount = 4, startPort = 8080) {
    if (workerCount < 1) {
      throw new Error("Worker count must be at least 1");
    }

    this.workers = [];
    for (let i = 0; i < workerCount; i++) {
      this.workers.push(new FitsWorker(startPort + i));
    }
  }

  get connectedWorkers() {
    return this.workers.filter((worker) => worker.connected);
  }

  get allConnected() {
    return this.workers.every(w=>w.connected);
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

  async getFileInfo(fileName: string, hdu: string) {
    return this.primaryWorker?.getFileInfo({ fileName, hdu });
  }

  async getSpectrum(fileName: string, hdu: string, x: number, y: number) {
    // TODO
  }
}
