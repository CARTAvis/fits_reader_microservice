import { WorkerPool } from "./workerPool/workerPool";

async function main() {
  const workerPool = new WorkerPool(4, 8080);
  console.time("getStatus");
  await workerPool.checkStatus();
  console.timeEnd("getStatus");
  let isOk = true;
  console.time("getFileInfo");
  // const promises = [];
  for (let i = 0; i < 1000; i++) {
    const inputFile = "myfits.fits";
    // promises.push(workerPool.getFileInfo(inputFile, "0").then(res=>isOk &&= res.fileName === inputFile));
    const fileInfoResponse = await workerPool.getFileInfo(inputFile, "0");
    isOk &&= fileInfoResponse.fileName === inputFile;
  }
  // await Promise.all(promises);
  console.timeEnd("getFileInfo");
  console.log(isOk ? "OK" : "ERROR");
  //console.log(fileInfoResponse);
}

main().catch(console.error);
