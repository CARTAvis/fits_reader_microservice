export function bytesToFloat32(bytes: Uint8Array) {
  if (bytes.byteOffset % 4 !== 0) {
    const copy = bytes.buffer.slice(bytes.byteOffset, bytes.byteOffset + bytes.length);
    return new Float32Array(copy, 0, bytes.length / 4);
  } else {
    return new Float32Array(bytes.buffer, bytes.byteOffset, bytes.length / 4);
  }
}

export function arrayStats(array: Float32Array) {
  let min = Number.MAX_VALUE;
  let max = Number.MIN_VALUE;
  let sum = 0;
  let sumSq = 0;
  let count = 0;

  for (let i = 0; i < array.length; i++) {
    const val = array[i];
    if (!isFinite(val)) {
      continue;
    }
    min = Math.min(min, val);
    max = Math.max(max, val);
    sum += val;
    sumSq += val * val;
    count++;
  }

  const mean = sum / count;
  const std = Math.sqrt(sumSq / count - mean * mean);

  return { min, max, mean, std, count };
}
