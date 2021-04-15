import manifest from './manifest.test.json'
import * as projections from './projections'
import { Vector2 } from './vectors'

function print(input, output) {
  console.log('input:\t', input, '\r\noutput:\t', output)
}

// eslint-disable-next-line no-unused-vars
function printCoords(input, output) {
  print(
    `(${input.x}, ${input.y}${input.z === null || input.z === undefined
      ? ''
      : `, ${input.z}`})`,
    `(${output.x}, ${output.y})`
  )
}

describe('projections.js', () => {
  // Always select the first camera
  let camera = manifest.cameraIntrinsicCalibration.filter(
    v => v.cameraUnderCalibration.name === 'camera0'
  )[0]

  describe('ProjectCalibratedPointToImage', () => {
    let x
    let y
    let calibratedPoint
    let focalLength
    let cameraCenter
    let kVals
    beforeEach(() => {
      // Using a random (x, y) for now
      x = 0
      y = 0

      calibratedPoint = new Vector2(x, y)
      focalLength = new Vector2(
        camera.scaledFocalLengthX,
        camera.scaledFocalLengthY
      )
      cameraCenter = new Vector2(camera.opticalCenterX, camera.opticalCenterY)
      kVals = [...camera.kannalaBrandt.radialDistortionCoefficientK]
    })

    it('should calculate the projected point on an image', () => {
      calibratedPoint = new Vector2(-1, 1)
      const result = projections.ProjectCalibratedPointToImage(
        calibratedPoint,
        focalLength,
        cameraCenter,
        kVals
      )

      expect(result.x).toBeTruthy()
      expect(result.y).toBeTruthy()
    })

    it('should return the pixel value of the optical center of the camera when (0, 0) is passed in as a calibrated point', () => {
      calibratedPoint = new Vector2(0, 0)
      const result = projections.ProjectCalibratedPointToImage(
        calibratedPoint,
        focalLength,
        cameraCenter,
        kVals
      )

      expect(result.x).toEqual(camera.opticalCenterX)
      expect(result.y).toEqual(camera.opticalCenterY)
    })
  })

  describe('UnprojectImageToCalibratedPoint', () => {
    let x
    let y
    let pixelPoint
    let focalLength
    let cameraCenter
    let kVals
    beforeEach(() => {
      // Using a random (x, y) for now
      x = 0
      y = 0

      pixelPoint = new Vector2(x, y)
      focalLength = new Vector2(
        camera.scaledFocalLengthX,
        camera.scaledFocalLengthY
      )
      cameraCenter = new Vector2(camera.opticalCenterX, camera.opticalCenterY)
      kVals = [...camera.kannalaBrandt.radialDistortionCoefficientK]
    })

    it('should return (0,0) when then optical center pixel value is provided as the calibrated point', () => {
      x = camera.opticalCenterX
      y = camera.opticalCenterY
      pixelPoint = new Vector2(x, y)

      const result = projections.UnprojectImageToCalibratedPoint(
        pixelPoint,
        focalLength,
        cameraCenter,
        kVals
      )

      expect(result.x).toBeCloseTo(0)
      expect(result.y).toBeCloseTo(0)
    })

    it('should project and unproject a random calibrated point correctly', () => {
      const calibratedPoint = new Vector2(-1.2341, 1.293472)

      const pixelPoint = projections.ProjectCalibratedPointToImage(
        calibratedPoint,
        focalLength,
        cameraCenter,
        kVals
      )
      const calibrated = projections.UnprojectImageToCalibratedPoint(
        pixelPoint,
        focalLength,
        cameraCenter,
        kVals
      )

      // console.log('calibrated input', calibratedPoint)
      // console.log('projected result', pixelPoint)
      // console.log('calibrated result', calibrated)
      expect(calibrated.x).toBeCloseTo(calibratedPoint.x)
      expect(calibrated.y).toBeCloseTo(calibratedPoint.y)
    })
  })
})
