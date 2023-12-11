import { WorkerPool } from "./workerPool/workerPool";
import { FileInfoResponse, ImageDataResponse } from "./proto/fitsReaderProto/file_service";
import { arrayStats } from "./utils/arrays";

async function main() {
  const numWorkers = 1;
  const workerPool = new WorkerPool(numWorkers, 8080);

  await workerPool.ready();

  console.time("getStatus");
  await workerPool.checkStatus();
  console.timeEnd("getStatus");
  let isOk = true;
  console.time("getFileInfo");

  let fileOpenResponse = await workerPool.openFile("/Users/angus/cubes/C.fits", "0");
  if (!fileOpenResponse?.uuid) {
    console.error("no uuid");
    return false;
  }

  const uuid = fileOpenResponse.uuid;

  // let fileInfoResponse: FileInfoResponse | undefined;
  // // const promises = [];
  // for (let i = 0; i < 1000; i++) {
  //   // promises.push(workerPool.getFileInfo(uuid).then(res=>isOk &&= res.fileName === inputFile));
  //   fileInfoResponse = await workerPool.getFileInfo(uuid);
  //   isOk &&= fileInfoResponse.hduShape?.[0] === 512;
  // }
  // // await Promise.all(promises);

  // console.timeEnd("getFileInfo");
  // console.log(isOk ? "OK" : "ERROR");
  // if (fileInfoResponse) {
  //   console.log(fileInfoResponse);
  // }

  const numImageTests = 32;
  let imageDataResponse: ImageDataResponse | undefined;

  // console.time("getImageDataParallel");
  // const promises = [];
  // for (let i = 0; i < numImageTests; i++) {
  //   promises.push(workerPool.getImageData(uuid, [1, 1, 1, 1], 512 * 512, i % numWorkers).then(res => imageDataResponse = res));
  //   //imageDataResponse = await workerPool.getImageData(uuid, "0", [1, 1, 1, 1], 512 * 512);
  // }
  // await Promise.all(promises);
  //
  // //const response = await workerPool.getImageData("/Users/angus/cubes/cosmos_spitzer3.6micron.fits", "0", [1, 1], 8209 * 8136);
  // console.timeEnd("getImageDataParallel");

  console.time("getImageData");
  for (let i = 0; i < numImageTests; i++) {
    imageDataResponse = await workerPool.getImageData(uuid, [1, 1, 1, 1], 512 * 512);
  }
  console.timeEnd("getImageData");

  await workerPool.closeFile(uuid);

  if (!imageDataResponse) {
    return;
  }

  fileOpenResponse = await workerPool.openFile("/Users/angus/cubes/fits/S255_IR_sci.spw29.cube.I.pbcor.fits", "0");
  if (!fileOpenResponse?.uuid) {
    console.error("no uuid");
    return false;
  }

  const uuid2 = fileOpenResponse.uuid;

  console.time("getSpectralProfile");
  const w = 128;
  const { data } = await workerPool.getSpectralProfile(uuid2, 968, 1033, 1, 1917, w, w, numWorkers);
  console.timeEnd("getSpectralProfile");
  console.log(arrayStats(data));

  await workerPool.closeFile(uuid2);
  return true;
}

main().catch(console.error);
