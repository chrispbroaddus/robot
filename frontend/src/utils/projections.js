/**
 * projections.js
 * =============================================================================
 * Utility functions & calculations to support the enhanced arc mode navigation
 */
import { Vector2 } from './vectors'

/**
 * Projects a calibrated point to an pixel-value image point
 *
 * This function is a port of ProjectCalibratedPointToImage(Vector2 calibratedPoint)
 * in /ZippySimUnity/Assets/ZippySimulator/Scripts/Calibration/KB4Camera.cs
 *
 * @param  {Vector2} calibratedPoint Calibrated (x,y) point
 * @param  {Vector2} focalLength     Focal length (x,y) of the camera
 * @param  {Vector2} cameraCenter    Optical center (x,y) of the camera
 * @param  {Array|Number} kVals      Kannala-Brandt radial distortion
 *                                   coefficients
 * @return {Vector2}                 Pixel point (x,y) for the image
 */
export const ProjectCalibratedPointToImage = (
  calibratedPoint,
  focalLength,
  cameraCenter,
  kVals
) => {
  const fu = focalLength.x
  const fv = focalLength.y
  const u0 = cameraCenter.x
  const v0 = cameraCenter.y

  const k0 = kVals[0]
  const k1 = kVals[1]
  const k2 = kVals[2]
  const k3 = kVals[3]

  const magnitude = Math.sqrt(
    Math.pow(calibratedPoint.x, 2) + Math.pow(calibratedPoint.y, 2)
  )
  const theta = Math.atan2(magnitude, 1)
  // Although psi is not necessarily needed, it protects us from dividing by 0
  // when magnitude == 0
  const psi = Math.atan2(calibratedPoint.y, calibratedPoint.x)

  const theta2 = theta * theta
  const theta3 = theta2 * theta
  const theta5 = theta3 * theta2
  const theta7 = theta5 * theta2
  const theta9 = theta7 * theta2
  const radius = theta + k0 * theta3 + k1 * theta5 + k2 * theta7 + k3 * theta9

  const pix = new Vector2(
    fu * radius * Math.cos(psi) + u0,
    fv * radius * Math.sin(psi) + v0
  )

  return pix
}

/**
 * Projects a calibrated point to an pixel-value image point
 *
 * This function is a port of UnprojectImageToCalibratedPoint(Vector2 pxPoint)
 * in /ZippySimUnity/Assets/ZippySimulator/Scripts/Calibration/KB4Camera.cs
 *
 * @param  {Vector2} pxPoint         Pixel image (x,y) point
 * @param  {Vector2} focalLength     Focal length (x,y) of the camera
 * @param  {Vector2} cameraCenter    Optical center (x,y) of the camera
 * @param  {Array|Number} kVals      Kannala-Brandt radial distortion
 *                                   coefficients
 * @return {Vector2}                 Calibrated point (x,y) for the camera
 */
export const UnprojectImageToCalibratedPoint = (
  pxPoint,
  focalLength,
  cameraCenter,
  kVals
) => {
  const fu = focalLength.x
  const fv = focalLength.y
  const u0 = cameraCenter.x
  const v0 = cameraCenter.y

  const k0 = kVals[0]
  const k1 = kVals[1]
  const k2 = kVals[2]
  const k3 = kVals[3]

  const un = pxPoint.x - u0
  const vn = pxPoint.y - v0
  const psi = Math.atan2(fu * vn, fv * un)

  const rth = un / (fu * Math.cos(psi))

  // Use Newtons method to solve for theta
  let th = rth
  for (let i = 0; i < 5; i++) {
    // f = (th + k0*th**3 + k1*th**5 + k2*th**7 + k3*th**9 - rth)^2
    const th2 = th * th
    const th3 = th2 * th
    const th4 = th2 * th2
    const th6 = th4 * th2
    const x0 =
      k0 * th3 + k1 * th4 * th + k2 * th6 * th + k3 * th6 * th3 - rth + th
    const x1 =
      3 * k0 * th2 + 5 * k1 * th4 + 7 * k2 * th6 + 9 * k3 * th6 * th2 + 1
    const d = 2 * x0 * x1
    const d2 =
      4 * th * x0 * (3 * k0 + 10 * k1 * th2 + 21 * k2 * th4 + 36 * k3 * th6) +
      2 * x1 * x1
    const delta = d / d2
    th -= delta
  }

  const z = Math.cos(th)
  const calibratedPoint = new Vector2(
    Math.sin(th) * Math.cos(psi) / z,
    Math.sin(th) * Math.sin(psi) / z
  )
  return calibratedPoint
}
