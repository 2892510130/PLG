## ArenaBuffer
- Compute the alignment: `(m_offset + alignment - 1) & ~(alignment - 1)`
  - The second part will give us 1000...000, always power of 2