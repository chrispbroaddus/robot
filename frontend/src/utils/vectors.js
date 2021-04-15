/**
 * vectors.js
 * =============================================================================
 * Vectors for use in geometric calculations for the arc mode
 */
/**
 * Two dimensional vector. Named Vector2 to mimic the conventions on the C# side
 */
export class Vector2 {
  constructor(x = null, y = null, magnitude = null) {
    this.x = x
    this.y = y
    this.magnitude = magnitude
  }
}

/**
 * Three dimensional vector. Named Vector3 to mimic the conventions on the C#
 * side
 */
export class Vector3 {
  constructor(x = null, y = null, z = null, magnitude = null) {
    this.x = x
    this.y = y
    this.z = z
    this.magnitude = magnitude
  }
}
