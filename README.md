# GPGPU Assignment

Gugi Gergely - 2024-04-14

## Task

N-Body simulation on the GPU.

### Methods:

- Barnes-Hut method
- Own implementation of octree on the GPU.

## Branches

- OcTree1:
  Base

- OcTree2:
  Instead of calculating Z indexes in the for loop, calculate it at the start and place the Nodes in the array accordingly. Therefore calculating the Z index becomes a single calculation instead of a START_DEPTH \* the same calculation inside the loop.
