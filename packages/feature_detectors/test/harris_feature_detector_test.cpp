
#include "packages/feature_detectors/include/harris_feature_detector.h"
#include "packages/feature_detectors/include/harris_feature_detector_details.h"
#include "gtest/gtest.h"

typedef core::DynamicStorage<core::uint8_scalar_t, core::AlignedMemoryAllocator> uint8_aligned_storage_t;
typedef core::DynamicStorage<core::int16_scalar_t, core::AlignedMemoryAllocator> int16_aligned_storage_t;
typedef core::UnmanagedStorage<core::int16_pixel_t> uint8_unmanaged_storage_t;
typedef core::UnmanagedStorage<core::int16_pixel_t> uint16_unmanaged_storage_t;

TEST(horizontalBinomial, regression) {
    const int result = feature_detectors::details::horizontalBinomial(1, 10, 20, 30, 40);
    EXPECT_EQ(321, result);
}

TEST(horizontalBinomial, minResult) {
    const int result = feature_detectors::details::horizontalBinomial(-255, -255, -255, -255, -255);
    EXPECT_EQ(-4080, result);
}

TEST(horizontalBinomial, maxResult) {
    const int result = feature_detectors::details::horizontalBinomial(255, 255, 255, 255, 255);
    EXPECT_EQ(4080, result);
}

TEST(verticalBinomial, regression) {
    const int result = feature_detectors::details::verticalBinomial(1, 10, 20, 30, 40);
    EXPECT_EQ(321, result);
}

TEST(verticalBinomial, minResult) {
    const int result = feature_detectors::details::verticalBinomial(-255, -255, -255, -255, -255);
    EXPECT_EQ(-4080, result);
}

TEST(verticalBinomial, maxResult) {
    const int result = feature_detectors::details::verticalBinomial(255, 255, 255, 255, 255);
    EXPECT_EQ(4080, result);
}

TEST(horizontalBinomialArray, regression) {
    const short x[] = { 1, 2, 3, 4, 5, 6, 7, 8, 9, 10 };

    int result[6];
    feature_detectors::details::horizontalBinomial(result, x + 2, 6);

    const int expected[] = { 48, 64, 80, 96, 112, 128 };

    for (int i = 0; i < 6; i++) {
        EXPECT_EQ(expected[i], result[i]);
    }
}

TEST(horizontalBinomialArray, minResult) {
    const short x[] = { -4080, -4080, -4080, -4080, -4080 };

    int result[1];
    feature_detectors::details::horizontalBinomial(result, x + 2, 1);

    const int expected[] = { -65280 };

    EXPECT_EQ(expected[0], result[0]);
}

TEST(horizontalBinomialArray, maxResult) {
    const short x[] = { 4080, 4080, 4080, 4080, 4080 };

    int result[1];
    feature_detectors::details::horizontalBinomial(result, x + 2, 1);

    const int expected[] = { 65280 };

    EXPECT_EQ(expected[0], result[0]);
}

TEST(verticalBinomialArray, regression) {
    const int src1[] = { 1, 2, 3 };
    const int src2[] = { 2, 3, 4 };
    const int src3[] = { 3, 4, 5 };
    const int src4[] = { 4, 5, 6 };
    const int src5[] = { 5, 6, 7 };

    int result[3];
    feature_detectors::details::verticalBinomial(result, src1, src2, src3, src4, src5, 3);

    EXPECT_EQ(result[0], 48);
    EXPECT_EQ(result[1], 64);
    EXPECT_EQ(result[2], 80);
}

TEST(verticalBinomialArray, minResult) {
    const int src1[] = { -4080 };
    const int src2[] = { -4080 };
    const int src3[] = { -4080 };
    const int src4[] = { -4080 };
    const int src5[] = { -4080 };

    int result[1];
    feature_detectors::details::verticalBinomial(result, src1, src2, src3, src4, src5, 1);

    const int expected[] = { -65280 };

    EXPECT_EQ(expected[0], result[0]);
}

TEST(verticalBinomialArray, maxResult) {
    const int src1[] = { 4080 };
    const int src2[] = { 4080 };
    const int src3[] = { 4080 };
    const int src4[] = { 4080 };
    const int src5[] = { 4080 };

    int result[1];
    feature_detectors::details::verticalBinomial(result, src1, src2, src3, src4, src5, 1);

    const int expected[] = { 65280 };

    EXPECT_EQ(expected[0], result[0]);
}

TEST(computeDerivatives, regression) {
    const unsigned char x1[] = { 0, 0, 0, 0 };
    const unsigned char x2[] = { 0, 255, 127, 0 };
    const unsigned char x3[] = { 0, 0, 0, 0 };

    short Ixx[2], Iyy[2], Ixy[2];
    feature_detectors::details::computeDerivatives(Ixx, Iyy, Ixy, x1 + 1, x2 + 1, x3 + 1, 2);

    EXPECT_EQ(3969, Ixx[0]);
    EXPECT_EQ(16129, Ixx[1]);
    EXPECT_EQ(0, Iyy[0]);
    EXPECT_EQ(0, Iyy[1]);
    EXPECT_EQ(0, Ixy[0]);
    EXPECT_EQ(0, Ixy[1]);
}

TEST(nonMaxSuppression5x5, pulseImageOneMaxima10x10) {
    float image[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

    core::ImageView<core::ImageType::float32> imageView(10, 10, 10, image);

    std::vector<feature_detectors::FeaturePoint> points;
    feature_detectors::nonMaxSuppression5x5(points, imageView);

    EXPECT_EQ(1, points.size());
    EXPECT_EQ(6, points[0].x());
    EXPECT_EQ(6, points[0].y());
    EXPECT_EQ(1, points[0].score());
}

TEST(nonMaxSuppression5x5, randomImageOneMaxima) {
    float image[] = { 71, 115, 184, 13, 150, 95, 173, 210, 31, 147, 158, 31, 170, 6, 23, 116, 14, 15, 183, 132, 226, 68, 168, 191, 255, 60,
        169, 134, 40, 235, 134, 146, 191, 210, 80, 253, 19, 15, 12, 108, 190, 41, 159, 187, 173, 157, 161, 188, 33 };

    core::ImageView<core::ImageType::float32> imageView(7, 7, 7, image);

    std::vector<feature_detectors::FeaturePoint> points;
    feature_detectors::nonMaxSuppression5x5(points, imageView);

    EXPECT_EQ(1, points.size());
    EXPECT_EQ(3, points[0].x());
    EXPECT_EQ(3, points[0].y());
}

TEST(nonMaxSuppression5x5, noValidPoints) {
    constexpr size_t size = 7;
    float image[size * size];
    core::ImageView<core::ImageType::float32> imageView(size, size, size, image);

    for (size_t i = 1; i < size - 1; i++) {
        for (size_t j = 1; j < size - 1; j++) {
            memset(imageView.data, 0, sizeof(float) * size * size);
            imageView.at(i, j) = 255;
            imageView.at(size / 2, size / 2) = 128;

            std::vector<feature_detectors::FeaturePoint> points;
            feature_detectors::nonMaxSuppression5x5(points, imageView);

            if (i == size / 2 && j == size / 2) {
                EXPECT_EQ(1, points.size());
            } else {
                EXPECT_EQ(0, points.size());
            }
        }
    }
}

/// Create 4 interest points in the extreme valid corners of the image and tests that harris discovers
/// the points with valid position and score.
TEST(HarrisFeatureDetection, fourPointRegression) {
    unsigned char image[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 255, 0, 0, 0, 0, 127, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 127, 0,
        0, 0, 0, 255, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0 };

    core::ImageView<core::ImageType::uint8> view(16, 16, 16, image);
    feature_detectors::HarrisFeatureDetector harris(0.06f);

    const std::vector<feature_detectors::FeaturePoint>& points = harris.detect(view);

    EXPECT_EQ(4, points.size());

    EXPECT_EQ(5, points[0].x());
    EXPECT_EQ(5, points[0].y());
    EXPECT_EQ(10, points[1].x());
    EXPECT_EQ(5, points[1].y());
    EXPECT_EQ(5, points[2].x());
    EXPECT_EQ(10, points[2].y());
    EXPECT_EQ(10, points[3].x());
    EXPECT_EQ(10, points[3].y());

    EXPECT_FLOAT_EQ(455523663872.000000, points[0].score());
    EXPECT_FLOAT_EQ(27584065536.000000, points[1].score());
    EXPECT_FLOAT_EQ(27584065536.000000, points[2].score());
    EXPECT_FLOAT_EQ(455523663872.000000, points[3].score());
}

TEST(HarrisFeatureDetection, matlabComparison) {
    unsigned char image[] = { 133, 100, 31, 127, 88, 52, 168, 115, 59, 45, 5, 242, 157, 107, 148, 199, 242, 180, 169, 50, 31, 139, 131, 39,
        108, 177, 182, 36, 94, 184, 196, 141, 43, 91, 144, 81, 169, 53, 233, 184, 135, 7, 210, 238, 216, 216, 240, 91, 249, 252, 110, 86,
        66, 3, 86, 180, 56, 20, 101, 7, 196, 219, 80, 129, 55, 30, 119, 45, 123, 97, 148, 166, 90, 149, 71, 5, 223, 72, 147, 114, 155, 80,
        221, 254, 251, 215, 91, 10, 141, 102, 84, 98, 149, 106, 28, 135, 38, 197, 244, 163, 217, 177, 181, 223, 163, 105, 198, 182, 108,
        144, 215, 166, 217, 233, 22, 178, 159, 1, 184, 117, 113, 182, 8, 168, 32, 183, 16, 237, 83, 2, 135, 28, 86, 70, 191, 200, 49, 9,
        210, 142, 32, 69, 25, 1, 153, 120, 20, 137, 118, 93, 155, 197, 97, 46, 134, 196, 232, 31, 189, 106, 109, 205, 208, 165, 84, 109, 38,
        146, 103, 232, 219, 51, 28, 238, 106, 191, 116, 186, 3, 67, 70, 181, 89, 144, 68, 151, 178, 123, 51, 140, 227, 206, 227, 194, 69,
        210, 127, 98, 7, 3, 187, 111, 100, 42, 126, 159, 35, 108, 4, 101, 182, 133, 70, 229, 83, 253, 79, 30, 17, 231, 250, 11, 77, 191,
        110, 203, 226, 5, 218, 162, 137, 90, 123, 182, 190, 71, 71, 194, 223, 165, 238, 106, 190, 46, 73, 240, 15, 195, 28, 144, 233, 17,
        195, 131, 164, 237 };
    float expectedStrength[] = { -1196263473152.000000, -1423994388480.000000, -816974659584.000000, -368398565376.000000,
        -287902203904.000000, -362785800192.000000, -397308985344.000000, -417000292352.000000, -717192429568.000000, -1550559739904.000000,
        -2084886806528.000000, -1517969473536.000000, -999464108032.000000, -1182830166016.000000, -1506026455040.000000,
        -1056982040576.000000, -1249692221440.000000, -1514855727104.000000, -966571851776.000000, -529335844864.000000,
        -441978814464.000000, -581169971200.000000, -767956615168.000000, -931288973312.000000, -1447592329216.000000,
        -2650976026624.000000, -3280411295744.000000, -2270556585984.000000, -1403655421952.000000, -1574881067008.000000,
        -2189314228224.000000, -1810606718976.000000, -1111017783296.000000, -1315398352896.000000, -822437609472.000000,
        -472781225984.000000, -455657390080.000000, -727675895808.000000, -1087327961088.000000, -1306624131072.000000,
        -1736711340032.000000, -2626259255296.000000, -3250222006272.000000, -2549510045696.000000, -1638643007488.000000,
        -1548110397440.000000, -1909347319808.000000, -1569451016192.000000, -1176671485952.000000, -1550620033024.000000,
        -1083587821568.000000, -745956966400.000000, -826763771904.000000, -1124195631104.000000, -1322000318464.000000,
        -1275398193152.000000, -1334393044992.000000, -1738535075840.000000, -2456298717184.000000, -2636912787456.000000,
        -2001692262400.000000, -1469704568832.000000, -1211327578112.000000, -769445134336.000000, -1085021618176.000000,
        -2001798168576.000000, -1881606455296.000000, -1483333959680.000000, -1578293526528.000000, -1806147649536.000000,
        -1586432049152.000000, -1078137847808.000000, -793020858368.000000, -907119820800.000000, -1537053294592.000000,
        -2065577279488.000000, -1703401488384.000000, -1064505835520.000000, -655531900928.000000, -349086777344.000000,
        -1507660136448.000000, -3233904328704.000000, -3033677430784.000000, -1981349888000.000000, -1851675639808.000000,
        -2085891997696.000000, -1627968110592.000000, -888910643200.000000, -528380035072.000000, -525783498752.000000,
        -864097206272.000000, -1201077878784.000000, -1036158500864.000000, -716777193472.000000, -589447233536.000000,
        -434949586944.000000, -2749429448704.000000, -4916395900928.000000, -3695252602880.000000, -2006661070848.000000,
        -1725004906496.000000, -1786677428224.000000, -1279512412160.000000, -760384520192.000000, -582759612416.000000,
        -552389246976.000000, -598667624448.000000, -701266198528.000000, -727169171456.000000, -777634840576.000000, -990877646848.000000,
        -846487552000.000000, -2652291727360.000000, -4118024552448.000000, -2738682593280.000000, -1547906973696.000000,
        -1381327175680.000000, -1292366381056.000000, -990282579968.000000, -938285924352.000000, -1143853416448.000000,
        -1180156166144.000000, -951906992128.000000, -856008818688.000000, -989875798016.000000, -1344568950784.000000,
        -1740526321664.000000, -1283597926400.000000, -1364736081920.000000, -2078730485760.000000, -1414143148032.000000,
        -862318493696.000000, -778701045760.000000, -794692812800.000000, -907490361344.000000, -1286080036864.000000,
        -1946772307968.000000, -2329834160128.000000, -2034189991936.000000, -1805258326016.000000, -1900944031744.000000,
        -2146237677568.000000, -2300113846272.000000, -1569259913216.000000, -1021386031104.000000, -1441253556224.000000,
        -948019462144.000000, -566209216512.000000, -413168336896.000000, -441002229760.000000, -682382917632.000000, -1064800026624.000000,
        -1803687559168.000000, -2621990764544.000000, -2818066350080.000000, -2893370884096.000000, -2857069969408.000000,
        -2435250651136.000000, -2137875152896.000000, -1564195422208.000000, -1377670660096.000000, -1615725854720.000000,
        -1035187519488.000000, -618634149888.000000, -346396491776.000000, -275005177856.000000, -403760054272.000000, -639686410240.000000,
        -1145619349504.000000, -1893834424320.000000, -2459408793600.000000, -2920775942144.000000, -2979246637056.000000,
        -2272971980800.000000, -1719949983744.000000, -1252806885376.000000, -1814764847104.000000, -2154722230272.000000,
        -1488959569920.000000, -868939333632.000000, -444370354176.000000, -293779046400.000000, -407596007424.000000, -709623545856.000000,
        -1056938328064.000000, -1489489887232.000000, -1988118970368.000000, -2523868430336.000000, -2790690652160.000000,
        -2348677332992.000000, -1698916597760.000000, -1009833082880.000000, -2025634660352.000000, -2713060638720.000000,
        -2199007526912.000000, -1429775712256.000000, -731979710464.000000, -450180874240.000000, -614320963584.000000,
        -1111225532416.000000, -1486261846016.000000, -1840773726208.000000, -2550486269952.000000, -3348988428288.000000,
        -3646714281984.000000, -3227214938112.000000, -2253922238464.000000, -989511155712.000000, -2352578822144.000000,
        -3279518171136.000000, -2849539096576.000000, -2111350505472.000000, -1234438979584.000000, -679519453184.000000,
        -650790961152.000000, -1061007785984.000000, -1677203079168.000000, -2588772925440.000000, -4005707120640.000000,
        -4910698463232.000000, -4661389557760.000000, -4033748926464.000000, -3016094646272.000000, -1318549192704.000000,
        -2474000252928.000000, -3425534476288.000000, -2965201747968.000000, -2422398517248.000000, -1846768959488.000000,
        -1061079613440.000000, -610128625664.000000, -672627818496.000000, -1430012821504.000000, -3058943393792.000000,
        -4526530101248.000000, -4382029250560.000000, -3558040928256.000000, -3406682128384.000000, -3276351733760.000000,
        -1848463327232.000000, -1327536275456.000000, -2081598210048.000000, -1906008915968.000000, -1749792980992.000000,
        -1715325239296.000000, -1063300235264.000000, -464375676928.000000, -355379052544.000000, -916814561280.000000,
        -2178109800448.000000, -2628271996928.000000, -1862691717120.000000, -1393676255232.000000, -1780492402688.000000,
        -2198667919360.000000, -1419664293888.000000 };

    core::ImageView<core::ImageType::uint8> imageView(16, 16, 16, image);
    core::ImageView<core::ImageType::float32> expectedStrengths(16, 16, 16, expectedStrength);
    feature_detectors::HarrisFeatureDetector harris(
        1.f); // we use k=1 here because c++/matlab give slightly different results with other numbers

    harris.detect(imageView);

    const core::DynamicImage<core::ImageType::float32, feature_detectors::HarrisFeatureDetector::float32_aligned_storage_t>& strengths
        = harris.strengths();

    const auto strengthsView = strengths.view();

    for (int i = 3; i < 13; i++) {
        for (int j = 3; j < 13; j++) {
            EXPECT_FLOAT_EQ(expectedStrengths.at(i, j), strengthsView.at(i, j)) << "invalid at " << i << " " << j;
        }
    }
}