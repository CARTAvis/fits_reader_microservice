import { WorkerPool } from "./workerPool/workerPool";
import { FileInfoResponse } from "./proto/fitsReaderProto/file_service";

async function main() {
  const workerPool = new WorkerPool(1, 8080);
  console.time("getStatus");
  await workerPool.checkStatus();
  console.timeEnd("getStatus");
  let isOk = true;
  console.time("getFileInfo");

  let fileInfoResponse: FileInfoResponse | undefined;
  // const promises = [];
  for (let i = 0; i < 1; i++) {
    const inputFile = "/Users/angus/cubes/C.fits";
    // promises.push(workerPool.getFileInfo(inputFile, "0").then(res=>isOk &&= res.fileName === inputFile));
    fileInfoResponse = await workerPool.getFileInfo(inputFile, "0");
    isOk &&= fileInfoResponse.fileName === inputFile && fileInfoResponse.hduShape?.[0] === 512;
  }
  // await Promise.all(promises);

  console.timeEnd("getFileInfo");
  console.log(isOk ? "OK" : "ERROR");
  if (fileInfoResponse) {
    console.log(fileInfoResponse);
  }
}

main().catch(console.error);
