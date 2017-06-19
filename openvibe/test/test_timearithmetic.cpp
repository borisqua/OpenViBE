/*
 * \author Jussi T. Lindgren / Inria
 * \date 29.03.2013
 * \brief Some tests for the Time Arithmetics class
 */
#include <iostream>
#include <iomanip>
#include <cstdlib> // abs() on Linux
#include <bitset>
#include <vector>

#include <openvibe/ovITimeArithmetics.h>

using namespace OpenViBE;
using namespace std;

// Legacy code to convert time to seconds
float64 oldTimeToSeconds(const uint64 ui64Time)
{
	return (ui64Time>>22)/1024.0;
}

// Legacy code to convert seconds to time
uint64 oldSecondsToTime(const float64 f64Time)
{
	return ((uint64)(f64Time*1024.0))<<22;
}

// legacy code from ovIStimulationSet.h for comparison
OpenViBE::uint64 sampleIndexToTime(OpenViBE::uint32 ui32SamplingRate, OpenViBE::uint64 ui64SampleIndex)
{	
	return ((ui64SampleIndex<<32)+(ui32SamplingRate-1))/ui32SamplingRate;
}

// legacy code from ovIStimulationSet.h for comparison
OpenViBE::uint64 timeToSampleIndex(OpenViBE::uint32 ui32SamplingRate, OpenViBE::uint64 ui64Time)
{
	return (ui64Time*ui32SamplingRate)>>32;
}

// Legacy code found from gdf writer box
OpenViBE::uint64 timeToSampleIndexGDFWriter(OpenViBE::uint32 ui32SamplingRate, OpenViBE::uint64 ui64Time)
{
	return (uint32)(((ui64Time + 1) * ui32SamplingRate - 1) >> 32);
}

// Alternate timeToSeconds and secondsToTime that very precise with decimals but not so great with huge numbers
//	f64seconds = float64(ui64Time)/float64(1LL<<32);
//	ui64time = uint64(f64Time*float64(1LL<<32)); 

int main(int argc, char *argv[])
{
	const float64 l_f64secsAndBackTolerance = 0.00001;

	// @note A rate of 101 can cause a glitch. Are we ever using uneven sampling rates?
	const uint32 samplingRatesToTest[] = { 100, 128, 512, 1000, 1024, 16000, 44100 };
	const float64 l_f64timesToTest[] = {
		0, 0.001, 0.01, 0.1, 0.2, 0.25, 0.5, 1.0, 1.1, 1.5, 2, 5, 10, 50, 100, 123.456789, 128.0, 500, 1000, 2500, 5000 };
	const uint64 l_ui64samplesToTest[] = { 0, 1, 100, 128, 512, 1000, 1021, 1024, 5005, 12345, 59876, 100000, 717893, 1000001 };
	const uint64 l_ui64timesToTest[] = { 1LL << 8, 1LL << 16, 1L << 19, 1LL << 22, 1LL << 27, 1L << 30, 1LL << 32, 10LL << 32, 100LL << 32, 123LL << 32, 500LL << 32, 512LL << 32, 1000LL << 32, 1024LL << 32, 2001LL << 32, 5000LL << 32 };

	cout << "Conversion tolerance from fixed point to float and back has been set to " << l_f64secsAndBackTolerance << " secs\n";
	cout << "ITimeArithmetics claims to support precision up to " << 1.0 / (1L << ITimeArithmetics::m_ui32DecimalPrecision) << " secs\n\n";

	int retVal = 0;

	{
		cout << "------\nTest: map time float->fixed->float\n---------\n";
		cout
			<< " " << setw(15) << "TimeFloat"
			<< " " << setw(15) << "Time"
			<< " " << setw(15) << "TimeFloatInv"
			<< " Tests\n";

		for (uint32 i = 0; i < sizeof(l_f64timesToTest) / sizeof(l_f64timesToTest[0]); i++)
		{
			uint64 l_ui64mapped = ITimeArithmetics::secondsToTime(l_f64timesToTest[i]);
			float64 l_f64inverse = ITimeArithmetics::timeToSeconds(l_ui64mapped);

			cout << " " << setw(15) << l_f64timesToTest[i] << " " << setw(15) << l_ui64mapped << " " << setw(15) << l_f64inverse;

			float64 l_f64AbsDiff = (l_f64timesToTest[i] - l_f64inverse);
			l_f64AbsDiff = (l_f64AbsDiff >= 0 ? l_f64AbsDiff : -l_f64AbsDiff); // no std::abs() for float64 on Ubuntu LTS gcc at the moment

			if (l_f64AbsDiff < l_f64secsAndBackTolerance)
			{
				cout << " Ok\n";
			}
			else if (l_f64AbsDiff < 1.0 / (1LL << ITimeArithmetics::m_ui32DecimalPrecision))
			{
				cout << " Ok(*)\n";
			}
			else {
				cout << " Err " << l_f64AbsDiff << "\n";
				retVal |= (1L << 0);
			}
		}
		cout << "*) Error is smaller than the minimum precision supported by ITimeArithmetics.h\n";
	}

	{
		cout << "------\nTest: map time fixed->float->fixed\n---------\n";
		cout
			<< " " << setw(15) << "Time"
			<< " " << setw(15) << "TimeFloat"
			<< " " << setw(15) << "TimeInv"
			<< " Tests\n";

		for (uint32 i = 0; i < sizeof(l_ui64timesToTest) / sizeof(l_ui64timesToTest[0]); i++)
		{
			uint64 l_ui64Time = l_ui64timesToTest[i];
			float64 l_f64mapped = ITimeArithmetics::timeToSeconds(l_ui64Time);
			uint64 l_ui64inverse = ITimeArithmetics::secondsToTime(l_f64mapped);

			cout << " " << setw(15) << l_ui64Time << " " << setw(15) << l_f64mapped << " " << setw(15) << l_ui64inverse;

			if (l_ui64inverse == l_ui64Time)
			{
				cout << " Ok\n";
			}
			else {
				uint64 l_ui64SignificantBitsA = (l_ui64inverse >> (32 - ITimeArithmetics::m_ui32DecimalPrecision));
				uint64 l_ui64SignificantBitsB = (l_ui64Time >> (32 - ITimeArithmetics::m_ui32DecimalPrecision));
				if (l_ui64SignificantBitsA == l_ui64SignificantBitsB) {
					cout << " Ok(*)\n";
				}
				else
				{
					cout << " Err\n";
					retVal |= (1L << 1);
				}
			}
		}
		cout << "*) Error is smaller than the minimum precision supported by ITimeArithmetics.h\n";
	}

	{
		cout << "------\nTest: time -> sample -> time using predefined times (orig. given as secs)\n------\n";

		cout << " " << setw(6) << "Rate"
			<< " " << setw(10) << "TimeSecs"
			<< " " << setw(15) << "Time"
			<< " " << setw(10) << "Sample"
			<< " " << setw(15) << "TimeInv"
			<< " Tests\n";
		for (size_t i = 0; i < sizeof(samplingRatesToTest) / sizeof(samplingRatesToTest[0]); i++) {
			for (size_t j = 0; j < sizeof(l_f64timesToTest) / sizeof(l_f64timesToTest[0]); j++) {

				float64 l_f64Time = l_f64timesToTest[j];

				uint64 l_ui64Time = ITimeArithmetics::secondsToTime(l_f64Time);
				uint64 sampleIdx = ITimeArithmetics::timeToSampleCount(samplingRatesToTest[i], l_ui64Time);
				uint64 timeFromSample = ITimeArithmetics::sampleCountToTime(samplingRatesToTest[i], sampleIdx);

				cout << " " << setw(6) << samplingRatesToTest[i]
					<< " " << setw(10) << l_f64Time
					<< " " << setw(15) << l_ui64Time
					<< " " << setw(10) << sampleIdx
					<< " " << setw(15) << timeFromSample
					;
				if (timeFromSample != l_ui64Time) {
					float64 l_f64Period = 1.0 / (float64)samplingRatesToTest[i];
					uint64 l_ui64AbsDiff = std::abs((int64)timeFromSample - (int64)l_ui64Time);
					float64 l_f64AbsDiff = ITimeArithmetics::timeToSeconds(l_ui64AbsDiff);
					if (l_f64AbsDiff > l_f64Period)
					{
						cout << " Error: Mismatch of " << l_f64AbsDiff << "s, period " << l_f64Period << "\n";
						retVal |= (1L << 2);
					}
					else {
						cout << " Ok(*)\n";
					}
				}
				else
				{
					cout << " Ok\n";
				}
			}
		}
		cout << "*) Error is smaller than precision allowed by the sampling rate\n";
	}

	{
		cout << "------\nTest: sample->time->sample using predefined samples\n------\n";

		cout << " " << setw(6) << "Rate"
			<< " " << setw(10) << "Sample"
			<< " " << setw(15) << "Time"
			<< " " << setw(10) << "SampleInv"
			<< " Tests\n";
		for (size_t i = 0; i < sizeof(samplingRatesToTest) / sizeof(samplingRatesToTest[0]); i++) {
			for (size_t j = 0; j < sizeof(l_ui64samplesToTest) / sizeof(l_ui64samplesToTest[0]); j++) {

				uint64 l_ui64testSample = l_ui64samplesToTest[j];
				uint64 timeFromSample = ITimeArithmetics::sampleCountToTime(samplingRatesToTest[i], l_ui64testSample);
				uint64 sampleInv = ITimeArithmetics::timeToSampleCount(samplingRatesToTest[i], timeFromSample);

				cout << " " << setw(6) << samplingRatesToTest[i]
					<< " " << setw(10) << l_ui64testSample
					<< " " << setw(15) << timeFromSample
					<< " " << setw(10) << sampleInv
					;
				if (l_ui64testSample == sampleInv)
				{
					cout << " Ok\n";
				}
				else
				{
					cout << " Error: Mismatch\n";
					retVal |= (1L << 3);
				}
			}
		}
	}


	{
		cout << "------\nTest: One second of signal at 'rate' should equal 'rate' samples\n------\n";
		cout << " " << setw(6) << "Rate"
			<< " " << setw(8) << "Samples"
			<< " Test\n";
		for (size_t i = 0; i < sizeof(samplingRatesToTest) / sizeof(samplingRatesToTest[0]); i++) {
			uint64 nSamples = ITimeArithmetics::timeToSampleCount(samplingRatesToTest[i], ITimeArithmetics::secondsToTime(1.0));
			cout << " " << setw(6) << samplingRatesToTest[i]
				<< " " << setw(8) << nSamples;
			if (nSamples == samplingRatesToTest[i]) {
				cout << " Ok\n";
			}
			else {
				cout << " Error\n";
				retVal |= (1L << 4);
			}
		}
	}

	cout << "------\nTest: Comparing epoch durations between ITimeArithmetics and old code from stimulationBasedEpoching\n---------\n";
	cout << " " << setw(10) << "Duration"
		<< " " << setw(15) << "ResultA"
		<< " " << setw(15) << "ResultB"
		<< " " << setw(10) << "AbsDiff"
		<< " Test\n";

	{
		const float64 l_f64EpochDurations[] = { 0.01, 0.1, 0.2, 0.25, 0.5, 1.0, 1.1, 1.5, 2, 5, 10, 50, 100 };
		for (uint32 i = 0; i < sizeof(l_f64EpochDurations) / sizeof(const float64); i++)
		{
			uint64 ui64resultA = (int64)(l_f64EpochDurations[i] * (1LL << 32)); // Code from stimulationBasedEpoching
			uint64 ui64resultB = ITimeArithmetics::secondsToTime(l_f64EpochDurations[i]);
			cout << " " << setw(10) << l_f64EpochDurations[i] << " " << setw(15) << ui64resultA << " " << setw(15) << ui64resultB;
			uint64 l_ui64AbsDiff = std::abs((int64)ui64resultA - int64(ui64resultB));
			float64 l_f64Diff = ITimeArithmetics::timeToSeconds(l_ui64AbsDiff);
			cout << " " << setw(10) << l_f64Diff;

			if (l_ui64AbsDiff == 0)
			{
				cout << " Ok\n";
			}
			else
			{
				uint64 l_ui64SignificantBitsA = (ui64resultA >> (32 - ITimeArithmetics::m_ui32DecimalPrecision));
				uint64 l_ui64SignificantBitsB = (ui64resultB >> (32 - ITimeArithmetics::m_ui32DecimalPrecision));
				if (l_ui64SignificantBitsA == l_ui64SignificantBitsB) {
					cout << " Ok(*)\n";
				}
				else {
					std::bitset<64> tmpA(ui64resultA);
					std::bitset<64> tmpB(ui64resultB);
					cout << " Err " << tmpA << " " << ITimeArithmetics::timeToSeconds(ui64resultA);
					cout << " " << tmpB << " " << ITimeArithmetics::timeToSeconds(ui64resultB) << "\n";
					retVal |= (1L << 5);
				}
			}
		}
		cout << "*) Error is smaller than the minimum precision supported by ITimeArithmetics.h\n";
	}

	cout << "------\nTest: Comparing timeToSampleCount variants\n---------\n";
	for (uint32 f = 0; f < sizeof(samplingRatesToTest) / sizeof(samplingRatesToTest[0]); f++) {
		const uint32 l_ui32TestFrequency = samplingRatesToTest[f];
		for (uint32 i = 0; i < 65537; i++) {
			const uint64 ui64resultA = ITimeArithmetics::timeToSampleCount(l_ui32TestFrequency, ITimeArithmetics::sampleCountToTime(l_ui32TestFrequency, i));
			const uint64 ui64resultB = timeToSampleIndexGDFWriter(l_ui32TestFrequency, ITimeArithmetics::sampleCountToTime(l_ui32TestFrequency, i));
			if (ui64resultA != ui64resultB)
			{
				cout << " Err " << l_ui32TestFrequency << " time idx " << i << " : " << ui64resultA << " vs " << ui64resultB << "\n";
				retVal |= (1L << 6);
			}
		}
	}
	if (!(retVal && (1L << 6)))
	{
		cout << "Ok\n";
	}

	cout << "------\nMisc tests (no errors will be reported)\n------------\n";

	uint64 ui64resultA, ui64resultB;

	// Test with milliseconds, the divisor 1000 can be in different positions in OV code. Small differences are to be expected here.
	const int ui32MilliSeconds = 123;
	ui64resultA = ((((uint64)ui32MilliSeconds)<<22)/1000)<<10;;		// Conversion that was sometimes used in the code
	ui64resultB = ITimeArithmetics::secondsToTime(ui32MilliSeconds/1000.0);
	cout << "m1: A " << ui64resultA << " B " << ui64resultB << "\n";

	// Code from stimulationBasedEpoching. Expect 0 diff
	const float64 l_f64EpochDuration = 1.1f;
	ui64resultA=(int64)(l_f64EpochDuration*(1LL<<32));
	ui64resultB=ITimeArithmetics::secondsToTime(l_f64EpochDuration);
	cout << "m2: A " << ui64resultA << " B " << ui64resultB << "\n";

	// Code from ovIStimulationSet.h. Time is typically off by one between the old code and code from ovITimeArithmetics.h using the following parameters.
	const uint64 l_ui64SampleIndex = 99; const uint32 l_ui32SamplingRate = 310;
	const uint64 l_ui64TestTime = 123LL<<32;
	ui64resultA = sampleIndexToTime(l_ui32SamplingRate, l_ui64SampleIndex); 
	ui64resultB = ITimeArithmetics::sampleCountToTime(l_ui32SamplingRate, l_ui64SampleIndex);
	cout << "m3: A " << ui64resultA << " B " << ui64resultB << "\n";
	ui64resultA = timeToSampleIndex(l_ui32SamplingRate, l_ui64TestTime);
	ui64resultB = ITimeArithmetics::timeToSampleCount(l_ui32SamplingRate, l_ui64TestTime);
	cout << "m4: A " << ui64resultA << " B " << ui64resultB << "\n";

	// Test a sequence of small indexes
	// Code from ovIStimulationSet.h. Time is typically off by one between the old code and code from ovITimeArithmetics.h using the following parameters.
	const uint32 l_ui32TestFrequency = 100;
	for(uint32 i=0;i<32;i++) {
		ui64resultA = sampleIndexToTime(l_ui32TestFrequency, i);
		ui64resultB = ITimeArithmetics::sampleCountToTime(l_ui32TestFrequency, i);
		cout << "m5: " << setw(3) << i << ": A " << ui64resultA << " B " << ui64resultB << "\n";
	}
	// Code from ovIStimulationSet.h. Sample counts should be equivalent regardless of the time skew of 1 earlier.
	for(uint32 i=0;i<32;i++) {
		ui64resultA = timeToSampleIndex(l_ui32TestFrequency, sampleIndexToTime(l_ui32TestFrequency, i));
		ui64resultB = ITimeArithmetics::timeToSampleCount(l_ui32TestFrequency, ITimeArithmetics::sampleCountToTime(100, i));
		cout << "m6: " << setw(3) << i << ": A " << ui64resultA << " B " << ui64resultB << "\n";
	}

	const float64 l_f64TestNumbers[] = { 1.23456789, 500.987654, 98765.123456, 123456789.123456789,
		12345678901234567890.1234567890} ;
	for(uint32 i=0;i<sizeof(l_f64TestNumbers)/sizeof(l_f64TestNumbers[0]);i++)
	{
		cout << "m7: " << setw(5) << i << " val " << setprecision(10) << " " << l_f64TestNumbers[i] << " A " << setprecision(10) << oldTimeToSeconds(oldSecondsToTime(l_f64TestNumbers[i]))
			<<  " B " << setprecision(10) << ITimeArithmetics::timeToSeconds(ITimeArithmetics::secondsToTime(l_f64TestNumbers[i]))
			<< "\n";
	}

#if 0
	// Steps the 32:32 timeline linearly, trying to convert to seconds and back
	const uint64 l_ui64Increment = ((1LL<<32)/1000)/4;		// 0.25 ms
	const uint64 l_ui64LowerBound = (0LL*60LL*60LL)<<32;	// 0 hours
	const uint64 l_ui64UpperBound = (48LL*60LL*60LL)<<32;	// 48 hours
	const uint64 l_ui64DumpInterval = l_ui64Increment*10000LL*60LL*10LL;		// 10 min

	uint64 l_ui64Errors = 0;
	uint64 l_ui64Tests = 0;

	for(uint64 i=l_ui64LowerBound; i<l_ui64UpperBound; i+=l_ui64Increment) {
		uint64 l_ui64floatAndBack = ITimeArithmetics::secondsToTime(ITimeArithmetics::timeToSeconds(i));

		uint64 l_ui64SignificantBitsA = (i >> (32-ITimeArithmetics::m_ui32DecimalPrecision));
		uint64 l_ui64SignificantBitsB = (l_ui64floatAndBack >> (32-ITimeArithmetics::m_ui32DecimalPrecision));
		if(l_ui64SignificantBitsA != l_ui64SignificantBitsB) {
			// cout << "Err " << setw(15) << i << " : " << (l_ui64floatAndBack-i) << "\n";
			l_ui64Errors++;
		}
		if( i == l_ui64Increment || i % l_ui64DumpInterval == 0) {
			cout << "m8: At " << i << " -> " << l_ui64floatAndBack << " ( " << ITimeArithmetics::timeToSeconds(i) << "s )\n";
		}
		l_ui64Tests++;
	}
	cout << "m8: Got " << l_ui64Errors << " errors in " << l_ui64Tests << " trials, " << l_ui64Errors/(float64)l_ui64Tests << "%\n";
#endif

#if 0
	// Randomized test
	uint64 l_ui64Errors = 0;
	for(uint64 i=0; i<1000000;i++) {
		uint64 l_ui64Random = System::Math::randomUInteger64() >> 10;

		if(l_ui64Random !=  ITimeArithmetics::secondsToTime(ITimeArithmetics::timeToSeconds(l_ui64Random))) {
	//		if(ITimeArithmetics::timeToSeconds(l_ui64Random)/(60.0*60.0)<500.0) {
			    cout << "Error converting " << l_ui64Random << " ( " << ITimeArithmetics::timeToSeconds(l_ui64Random)/(60.0*60.0) << " h)\n";
	//		}
			l_ui64Errors++;
		}
	}
	cout << "Got " << l_ui64Errors << " errors\n";
#endif

	std::cout << "Test mapping 1ms from fixed point time.\n";

	// Old openvibe computation in getTime: (uint32)(((time>>22)*1000)>>10);
	//
    //        8       16       24       32       40       48       56       64
	// ZZZZZZZZ ZZXXXXXX XXXXXXXX XXXXXXXX YYYYYYYY YYYYYYYY YYYYYYYY YYYYYYYY >> 22
	// 00000000 00000000 000000ZZ ZZZZZZZZ XXXXXXXX XXXXXXXX XXXXXXYY YYYYYYYY << 10 (= approx * 1000 (2^10=1024))
	// 00000000 0000ZZZZ ZZZZZZXX XXXXXXXX XXXXXXXX XXXXYYYY YYYYYY00 00000000 >> 10
	// 00000000 00000000 000000ZZ ZZZZZZZZ XXXXXXXX XXXXXXXX XXXXXXYY YYYYYYYY 
	//
	// The interpretation of this operation. Mathematically we want to get a 
	// 64 bits fixed time in milliseconds -> multiply by 1000. Since this 
	// would cause the most significant bits of the 64bit fixed point to overflow,
	// we first arrange some headroom with >> 22. Then multiply with 10. 
	// Last shift with 10 brings the millisecond part to the LSB of 32 bits.
	// Note that Z will be discarded by the cast to 32bit. So effectively
	// this approximates a 32bit shift of a 64bit fixed point value multiplied 
	// by 1000.
	//
	// The simple way (time*1000)>>32,
	//
    // -16      -8                8       16       24       32       40       48       56       64	    
	// 00000000 00000000 | ZZZZZZZZ ZZXXXXXX XXXXXXXX XXXXXXXX YYYYYYYY YYYYYYYY YYYYYYYY YYYYYYYY << 10 (= approx * 1000 (2^10=1024))
	// 000000ZZ ZZZZZZZZ | XXXXXXXX XXXXXXXX XXXXXXYY YYYYYYYY YYYYYYYY YYYYYYYY YYYYYY00 00000000 >> 32
	// 00000000 00000000 | 00000000 00000000 00000000 00000000 XXXXXXXX XXXXXXXX XXXXXXYY YYYYYYYY		// due to truncation
	//
	// The difference is that the first can be more accurate with high-order bits, and the latter
	// with small order bits?
	// 
	// Another variant, ((ui64Seconds>>10)*1000)>>22)),
	//
    //        8       16       24       32       40       48       56       64
	// ZZZZZZZZ ZZXXXXXX XXXXXXXX XXXXXXXX YYYYYYYY YYYYYYYY YYYYYYYY YYYYYYYY >> 10
	// 00000000 00ZZZZZZ ZZZZXXXX XXXXXXXX XXXXXXXX XXYYYYYY YYYYYYYY YYYYYYYY << 10 (= approx * 1000 (2^10=1024))
	// ZZZZZZZZ ZZXXXXXX XXXXXXXX XXXXXXXX YYYYYYYY YYYYYYYY YYYYYY00 00000000 >> 22
	// 00000000 00000000 000000ZZ ZZZZZZZZ XXXXXXXX XXXXXXXX XXXXXXYY YYYYYYYY (uint32)
	// 00000000 00000000 00000000 00000000 XXXXXXXX XXXXXXXX XXXXXXYY YYYYYYYY
	// 
	// The difference: when multiplying by 1000, 
	//
	// 1) the first uses 10 most significant fraction bits. 
	// 2) The second uses all of them. 
	// 3) the third uses 22.
	// 
	// Tests with 1ms precision:
	//
	// 0x800000 is the first 64bit number method1 understands as "1 ms".
	// 0x418938 -"-                       method2 -""-
	// 0x418c00 -"-                       method3 -""-
	// 
	// 1ms obtained by (1LL<<32)/1000LL is                0x418937 -> about 2x difference to method 1
	// 1ms obtained by ov secondstoTime is                0x400000
	// 1ms obtained by (2^32)/1000 is       4294967.296 = 0x418937 + 0.296
	// 1ms obtained by (2^32-1)/1000 is     4294967.295 = 0x418937 + 0.295
	//

	static const uint64 l_ui64Expected = 4294967; // + 0.296

	std::cout << "As fixed point, its " << l_ui64Expected   << " (expected), "
		                                << (1LL<<32)/1000LL << " (naive) or " 
										<< ITimeArithmetics::secondsToTime(0.001) << " (timeArithm)\n";

	for(uint64 i=0;i<10000000;i++) {
		const uint32 l_ui32InMillis= (uint32)(((i>>22)*1000)>>10);

		if(l_ui32InMillis == 1) {
			std::cout << "Method 1 : 1ms first at " << i << "\n";
			break;
		}
	}

	for(uint64 i=0;i<10000000;i++) {
		const uint32 l_ui32InMillis = (uint32)((i * 1000) >> 32);

		if(l_ui32InMillis == 1) {
			std::cout << "Method 2 : 1ms first at " << i << "\n";
			break;
		}
	}

	for(uint64 i=0;i<10000000;i++) {
		const uint32 l_ui32InMillis = (uint32)(((i>>10)*1000) >> 22);

		if(l_ui32InMillis == 1) {
			std::cout << "Method 3 : 1ms first at " << i << "\n";
			break;
		}
	}

	/*
	uint64 test = (1LL << 32);
	uint64 test2 = (128LL << 32);
	uint64 test3 = (128LL << 32) + (1LL << 31);

	std::cout << ITimeArithmetics::timeToSeconds(test/2) << "\n";
	std::cout << ITimeArithmetics::timeToSeconds(test/512) << "\n";
	std::cout << ITimeArithmetics::timeToSeconds(test2/512) << "\n";
	std::cout << ITimeArithmetics::timeToSeconds(test3) << "->" << ITimeArithmetics::timeToSeconds(test3/10) << "\n";
	*/

	// Fraction test, converting <secs,ms> pair to 32:32 fixed point

	const uint64 microsInSecond = 1000*1000;

	std::vector< std::pair<uint64,uint64> > tmp;
	tmp.push_back(std::pair<uint64,uint64>(0LL,0LL));                    // 0s
	tmp.push_back(std::pair<uint64,uint64>(0LL,1LL));                    // 1s + 1us
	tmp.push_back(std::pair<uint64,uint64>(0LL,2LL));
	tmp.push_back(std::pair<uint64,uint64>(0LL,(microsInSecond / 2LL))); // 500ms
	tmp.push_back(std::pair<uint64,uint64>(0LL,microsInSecond-2));
	tmp.push_back(std::pair<uint64,uint64>(0LL,microsInSecond-1));       // 1 step below 1s
	tmp.push_back(std::pair<uint64,uint64>(1LL,0LL));                    // 1s
	tmp.push_back(std::pair<uint64,uint64>(1LL,1LL));                    // 1 step above 1s
	tmp.push_back(std::pair<uint64,uint64>(0LL,microsInSecond+1));       // same as (1LL,1LL), but shouldn't happen as micro fraction is meant to be capped under 1s
	tmp.push_back(std::pair<uint64,uint64>(1LL,2LL));
	tmp.push_back(std::pair<uint64,uint64>(1LL,1000LL));                 // 1s + 1ms
	tmp.push_back(std::pair<uint64,uint64>(1LL,2000LL));                 // 1s + 2ms

	for(size_t i=0;i<tmp.size();i++) 
	{
		const uint64 l_ui64FixedPoint = ITimeArithmetics::subsecondsToTime(tmp[i].first, tmp[i].second, microsInSecond);
		cout << "m9 pair (" << tmp[i].first << "s, " << tmp[i].second << "us) as float=" 
			<< ITimeArithmetics::timeToSeconds( l_ui64FixedPoint ) << "s, parts secs=" 
			<< (l_ui64FixedPoint>>32) << " fract=" << (l_ui64FixedPoint & 0xFFFFFFFFLL) << "\n";
	}

	// These should be approx equal
	cout << "m9 Diff 1 " << ITimeArithmetics::subsecondsToTime(tmp[1].first,tmp[1].second,microsInSecond)-ITimeArithmetics::subsecondsToTime(tmp[0].first,tmp[0].second,microsInSecond) << "\n";
	cout << "m9 Diff 1 " << ITimeArithmetics::subsecondsToTime(tmp[6].first,tmp[6].second,microsInSecond)-ITimeArithmetics::subsecondsToTime(tmp[5].first,tmp[5].second,microsInSecond) << "\n";
	cout << "m9 Diff 1 " << ITimeArithmetics::subsecondsToTime(tmp[7].first,tmp[7].second,microsInSecond)-ITimeArithmetics::subsecondsToTime(tmp[6].first,tmp[6].second,microsInSecond) << "\n";

	return retVal;
} 