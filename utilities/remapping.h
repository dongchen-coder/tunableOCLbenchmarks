
__device__ int remappingThreadIDx(int tidx, int mode) {
	if (mode == 1) {
		return (tidx + 32) % blockDim.x;
	} else if (mode == 2) { 
		return (tidx + blockDim.x / 2 / 32 * 32 ) % blockDim.x;
	} else if (mode == 3) {
		return (tidx + blockDim.x / 4 / 32 * 32 ) % blockDim.x;
	} else if (mode == 4) {
		return (tidx + blockDim.x * 3 / 4 / 32 * 32 ) % blockDim.x;
	} else if (mode == 5) {
		return blockDim.x - 1 - tidx;
	} else if (mode == 6) {
		return (blockDim.x - 1 - tidx + blockDim.x / 2 / 32 * 32 ) % blockDim.x;
	} else if (mode == 7) {
		return (blockDim.x - 1 - tidx + blockDim.x / 4 / 32 * 32 ) % blockDim.x;
	} else if (mode == 8) {
		return (blockDim.x - 1 - tidx + blockDim.x * 3 / 4 / 32 * 32 ) % blockDim.x;
	}
	return tidx;
}

__device__ int remappingThreadIDy(int tidy, int mode) {
	if (mode == 1) {
        return (tidy + 32) % blockDim.y;
    } else if (mode == 2) {
        return (tidy + blockDim.y / 2 / 32 * 32 ) % blockDim.y;
    } else if (mode == 3) {
        return (tidy + blockDim.y / 4 / 32 * 32 ) % blockDim.y;
    } else if (mode == 4) {
        return (tidy + blockDim.y * 3 / 4 / 32 * 32 ) % blockDim.y;
    } else if (mode == 5) {
        return blockDim.y - 1 - tidy;
    } else if (mode == 6) {
        return (blockDim.y - 1 - tidy + blockDim.y / 2 / 32 * 32 ) % blockDim.y;
    } else if (mode == 7) {
        return (blockDim.y - 1 - tidy + blockDim.y / 4 / 32 * 32 ) % blockDim.y;
    } else if (mode == 8) {
        return (blockDim.y - 1 - tidy + blockDim.y * 3 / 4 / 32 * 32 ) % blockDim.y;
    }
	return tidy;
}

__device__ int remappingBlockIDx(int bidx, int mode) {
	if (mode == 1) {
		return (bidx + 1) % gridDim.x;
	} else if (mode == 2) {
		return (bidx + gridDim.x / 2) % gridDim.x;
	} else if (mode == 3) {
		return (bidx + gridDim.x / 4) % gridDim.x;
	} else if (mode == 4) {
		return (bidx + gridDim.x * 3 / 4) % gridDim.x;
	} else if (mode == 5) {
		return gridDim.x - 1 - bidx;
	} else if (mode == 6) {
		return (gridDim.x - 1 - bidx + gridDim.x / 2) % gridDim.x;
	} else if (mode == 7) {
		return (gridDim.x - 1 - bidx + gridDim.x / 4) % gridDim.x;
	} else if (mode == 8) {
		return (gridDim.x - 1 - bidx + gridDim.x * 3 / 4) % gridDim.x;
	}
	return bidx;
}

__device__ int remappingBlockIDy(int bidy, int mode) {
	if (mode == 1) {
        return (bidy + 1) % gridDim.y;
    } else if (mode == 2) {
        return (bidy + gridDim.y / 2) % gridDim.y;
    } else if (mode == 3) {
        return (bidy + gridDim.y / 4) % gridDim.y;
    } else if (mode == 4) {
        return (bidy + gridDim.y * 3 / 4) % gridDim.y;
    } else if (mode == 5) {
        return gridDim.y - 1 - bidy;
    } else if (mode == 6) {
        return (gridDim.y - 1 - bidy + gridDim.y / 2) % gridDim.y;
    } else if (mode == 7) {
        return (gridDim.y - 1 - bidy + gridDim.y / 4) % gridDim.y;
    } else if (mode == 8) {
        return (gridDim.y - 1 - bidy + gridDim.y * 3 / 4) % gridDim.y;
    }
	return bidy;
}


