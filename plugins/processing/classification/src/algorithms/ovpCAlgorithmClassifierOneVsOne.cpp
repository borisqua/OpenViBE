#include "ovpCAlgorithmClassifierOneVsOne.h"
#include "ovpCAlgorithmPairwiseDecision.h"

#include <map>
#include <cmath>
#include <sstream>
#include <cstring>
#include <cstdlib>
#include <string>
#include <utility>
#include <iostream>
#include <system/ovCMemory.h>

namespace{
	const char* const c_sTypeNodeName = "OneVsOne";
	const char* const c_sSubClassifierIdentifierNodeName = "SubClassifierIdentifier";
	const char* const c_sPairwiseDecisionName = "PairwiseDecision";
	const char* const c_sAlgorithmIdAttribute = "algorithm-id";
	const char* const c_sSubClassifierCountNodeName = "SubClassifierCount";
	const char* const c_sSubClassifiersNodeName = "SubClassifiers";
	const char* const c_sSubClassifierNodeName = "SubClassifier";

	const char* const c_sFirstClassAtrributeName = "first-class";
	const char* const c_sSecondClassAttributeName = "second-class";

	//This map is used to record the decision strategies available for each algorithm
	std::map<OpenViBE::uint64, OpenViBE::CIdentifier> g_oDecisionMap;
}

extern const char* const c_sClassifierRoot;

using namespace OpenViBE;
using namespace OpenViBE::Kernel;
using namespace OpenViBE::Plugins;

using namespace OpenViBEPlugins;
using namespace OpenViBEPlugins::Classification;

using namespace OpenViBEToolkit;


void OpenViBEPlugins::Classification::registerAvailableDecisionEnumeration(const CIdentifier& rAlgorithmIdentifier, CIdentifier pDecision)
{
	g_oDecisionMap[rAlgorithmIdentifier.toUInteger()] = pDecision;
}

CIdentifier OpenViBEPlugins::Classification::getAvailableDecisionEnumeration(const OpenViBE::CIdentifier& rAlgorithmIdentifier)
{
	if(g_oDecisionMap.count(rAlgorithmIdentifier.toUInteger()) == 0)
		return OV_UndefinedIdentifier;
	else
		return g_oDecisionMap[rAlgorithmIdentifier.toUInteger()];
}

boolean CAlgorithmClassifierOneVsOne::initialize()
{
	TParameterHandler < XML::IXMLNode* > op_pConfiguration(this->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Configuration));
	op_pConfiguration=NULL;

	TParameterHandler < uint64 > ip_pPairwise(this->getInputParameter(OVP_Algorithm_OneVsOneStrategy_InputParameterId_DecisionType));
	ip_pPairwise = OV_UndefinedIdentifier.toUInteger();

	m_pDecisionStrategyAlgorithm = NULL;
	m_oPairwiseDecisionIdentifier = OV_UndefinedIdentifier;

	return CAlgorithmPairingStrategy::initialize();
}

boolean CAlgorithmClassifierOneVsOne::uninitialize(void)
{
	if(m_pDecisionStrategyAlgorithm != NULL)
	{
		m_pDecisionStrategyAlgorithm->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pDecisionStrategyAlgorithm);
		m_pDecisionStrategyAlgorithm = NULL;
	}

	std::map< std::pair<uint32, uint32>, IAlgorithmProxy* >::iterator it;
	for(it = m_oSubClassifiers.begin(); it != m_oSubClassifiers.end(); it++)
	{
		IAlgorithmProxy* l_pSubClassifier = (*it).second;
		l_pSubClassifier->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*l_pSubClassifier);
	}
	this->m_oSubClassifiers.clear();

	return CAlgorithmPairingStrategy::uninitialize();
}



boolean CAlgorithmClassifierOneVsOne::train(const IFeatureVectorSet& rFeatureVectorSet)
{

	TParameterHandler < uint64 > ip_pNumberOfClasses(this->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_NumberOfClasses));
	m_ui32NumberOfClasses = static_cast<uint32>(ip_pNumberOfClasses);

	m_ui32NumberOfSubClassifiers = m_ui32NumberOfClasses*(m_ui32NumberOfClasses-1)/2;

	createSubClassifiers();

	//Create the decision strategy
	this->initializeExtraParameterMechanism();

	m_oPairwiseDecisionIdentifier=this->getEnumerationParameter(OVP_Algorithm_OneVsOneStrategy_InputParameterId_DecisionType, OVP_TypeId_ClassificationPairwiseStrategy);

	if(m_oPairwiseDecisionIdentifier==OV_UndefinedIdentifier) {
		this->getLogManager() << LogLevel_Error << "Tried to get algorithm id for pairwise decision strategy " << OVP_TypeId_ClassificationPairwiseStrategy << " but failed\n";
		return false;
	}
	
	if(m_pDecisionStrategyAlgorithm != NULL){
		m_pDecisionStrategyAlgorithm->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*m_pDecisionStrategyAlgorithm);
		m_pDecisionStrategyAlgorithm = NULL;
	}
	m_pDecisionStrategyAlgorithm = &this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(m_oPairwiseDecisionIdentifier));
	m_pDecisionStrategyAlgorithm->initialize();

	TParameterHandler < CIdentifier *> ip_pClassificationAlgorithm(m_pDecisionStrategyAlgorithm->getInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameterId_AlgorithmIdentifier));
	ip_pClassificationAlgorithm = &m_oSubClassifierAlgorithmIdentifier;
	TParameterHandler <uint64> ip_pClassCount(m_pDecisionStrategyAlgorithm->getInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameter_ClassCount));
	ip_pClassCount = m_ui32NumberOfClasses;

	m_pDecisionStrategyAlgorithm->process(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_Parameterize);

	this->uninitializeExtraParameterMechanism();


	//Calculate the amount of sample for each class
	std::map < float64, size_t > l_vClassLabels;
	for(uint32 i=0; i<rFeatureVectorSet.getFeatureVectorCount(); i++)
	{
		if(!l_vClassLabels.count(rFeatureVectorSet[i].getLabel()))
		{
			l_vClassLabels[rFeatureVectorSet[i].getLabel()] = 0;
		}
		l_vClassLabels[rFeatureVectorSet[i].getLabel()]++;
	}

	if(l_vClassLabels.size() != m_ui32NumberOfClasses)
	{
		this->getLogManager() << LogLevel_Error << "There are samples for " << (uint32)l_vClassLabels.size() << " classes but expected samples for " << m_ui32NumberOfClasses << " classes.\n";
		return false;
	}

	//Now we create the corresponding repartition set
	TParameterHandler<IMatrix*> ip_pRepartitionSet = m_pDecisionStrategyAlgorithm->getInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameterId_SetRepartition);
	ip_pRepartitionSet->setDimensionCount(1);
	ip_pRepartitionSet->setDimensionSize(0, m_ui32NumberOfClasses);

	const size_t l_iFeatureVectorSize=rFeatureVectorSet[0].getSize();
	//Now let's train each classifier
	for(size_t l_iFirstClass=0 ; l_iFirstClass < m_ui32NumberOfClasses; ++l_iFirstClass)
	{
		ip_pRepartitionSet->getBuffer()[l_iFirstClass] = l_vClassLabels[static_cast<float64>(l_iFirstClass)];

		for(size_t l_iSecondClass = l_iFirstClass+1 ; l_iSecondClass < m_ui32NumberOfClasses ; ++l_iSecondClass)
		{
			size_t l_iFeatureCount = l_vClassLabels[(float64)l_iFirstClass] + l_vClassLabels[static_cast<float64>(l_iSecondClass)];

			IAlgorithmProxy* l_pSubClassifier = m_oSubClassifiers[std::pair<uint32,uint32>(l_iFirstClass, l_iSecondClass)];

			TParameterHandler < IMatrix* > ip_pFeatureVectorSet(l_pSubClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_FeatureVectorSet));
			ip_pFeatureVectorSet->setDimensionCount(2);
			ip_pFeatureVectorSet->setDimensionSize(0, l_iFeatureCount);
			ip_pFeatureVectorSet->setDimensionSize(1, l_iFeatureVectorSize+1);

			float64* l_pFeatureVectorSetBuffer=ip_pFeatureVectorSet->getBuffer();
			for(size_t j=0; j<rFeatureVectorSet.getFeatureVectorCount(); j++)
			{
				const float64 l_f64TempClass = rFeatureVectorSet[j].getLabel();
				if(l_f64TempClass == static_cast<float64>(l_iFirstClass) || l_f64TempClass == static_cast<float64>(l_iSecondClass))
				{
					System::Memory::copy(
								l_pFeatureVectorSetBuffer,
								rFeatureVectorSet[j].getBuffer(),
								l_iFeatureVectorSize*sizeof(float64));

					if(static_cast<size_t>(l_f64TempClass) == l_iFirstClass )
					{
						l_pFeatureVectorSetBuffer[l_iFeatureVectorSize]=0;
					}
					else
					{
						l_pFeatureVectorSetBuffer[l_iFeatureVectorSize]=1;
					}
					l_pFeatureVectorSetBuffer+=(l_iFeatureVectorSize+1);
				}
			}
			l_pSubClassifier->process(OVTK_Algorithm_Classifier_InputTriggerId_Train);
		}
	}
	return true;
}

boolean CAlgorithmClassifierOneVsOne::classify(const IFeatureVector& rFeatureVector, float64& rf64Class, IVector& rClassificationValues, IVector& rProbabilityValue)
{
	if(!m_pDecisionStrategyAlgorithm) {
		this->getLogManager() << LogLevel_Error << "Called without decision strategy algorithm\n";
		return false;
	}

	const uint32 l_ui32FeatureVectorSize=rFeatureVector.getSize();
	std::vector< SClassificationInfo > l_oClassificationList;

	TParameterHandler<IMatrix*> ip_pProbabilityMatrix = m_pDecisionStrategyAlgorithm->getInputParameter(OVP_Algorithm_Classifier_InputParameter_ProbabilityMatrix);
	IMatrix * l_pProbabilityMatrix = (IMatrix*)ip_pProbabilityMatrix;

	l_pProbabilityMatrix->setDimensionCount(2);
	l_pProbabilityMatrix->setDimensionSize(0, m_ui32NumberOfClasses);
	l_pProbabilityMatrix->setDimensionSize(1, m_ui32NumberOfClasses);

	for(OpenViBE::uint32 i =0; i < l_pProbabilityMatrix->getBufferElementCount() ; ++i)
	{
		l_pProbabilityMatrix->getBuffer()[i] = 0.0;
	}

	//Let's generate the matrix of confidence score
	for(OpenViBE::uint32 i =0; i< m_ui32NumberOfClasses ; ++i)
	{
		for(OpenViBE::uint32 j = i+1 ; j< m_ui32NumberOfClasses ; ++j)
		{
			IAlgorithmProxy * l_pTempProxy = m_oSubClassifiers[std::pair<uint32,uint32>(i, j)];
			TParameterHandler < IMatrix* > ip_pFeatureVector(l_pTempProxy->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_FeatureVector));
			TParameterHandler < IMatrix* > op_pClassificationValues(l_pTempProxy->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_ProbabilityValues));
			TParameterHandler < float64 > op_pClassificationLabel (l_pTempProxy->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Class));
			ip_pFeatureVector->setDimensionCount(1);
			ip_pFeatureVector->setDimensionSize(0, l_ui32FeatureVectorSize);

			float64* l_pFeatureVectorBuffer=ip_pFeatureVector->getBuffer();
			System::Memory::copy(
						l_pFeatureVectorBuffer,
						rFeatureVector.getBuffer(),
						l_ui32FeatureVectorSize*sizeof(float64));
			l_pTempProxy->process(OVTK_Algorithm_Classifier_InputTriggerId_Classify);

			SClassificationInfo l_oClassificationInfo = {(float64)i, (float64)j, op_pClassificationLabel, op_pClassificationValues};
			l_oClassificationList.push_back(l_oClassificationInfo);
		}
	}

//	for(size_t i =0 ; i < l_ui32ClassCount ; ++i )
//	{
//		for(size_t j = 0; j < l_ui32ClassCount ; ++j)
//		{
//			std::cout << l_pProbabilityMatrix->getBuffer()[i*l_ui32ClassCount + j] << " " ;
//		}
//		std::cout << std::endl;
//	}
//	std::cout << std::endl;

	TParameterHandler< std::vector < SClassificationInfo > * > ip_pClassificationInfos(
				m_pDecisionStrategyAlgorithm->getInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameter_ClassificationOutputs));
	ip_pClassificationInfos = &l_oClassificationList;

	//Then ask to the startegy to make the decision
	if(!m_pDecisionStrategyAlgorithm->process(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_Compute))
	{
		this->getLogManager() << LogLevel_Error << "Decision strategy compute() failed.\n";
		return false;
	}

	TParameterHandler<IMatrix*> op_pProbabilityVector = m_pDecisionStrategyAlgorithm->getOutputParameter(OVP_Algorithm_Classifier_OutputParameter_ProbabilityVector);
	float64 l_f64MaxProb = -1;
	int32 l_i32IndexSelectedClass = -1;

	rClassificationValues.setSize(0);
	rProbabilityValue.setSize(m_ui32NumberOfClasses);

	//We just have to take the most relevant now.
	for(uint32 i = 0 ; i < m_ui32NumberOfClasses ; ++i)
	{
		const float64 l_f64TempProb = op_pProbabilityVector->getBuffer()[i];
		if(l_f64TempProb > l_f64MaxProb)
		{
			l_i32IndexSelectedClass = i;
			l_f64MaxProb = l_f64TempProb;
		}
		rProbabilityValue[i] = l_f64TempProb;
	}

	rf64Class = static_cast<float64>(l_i32IndexSelectedClass);
	return true;
}

boolean CAlgorithmClassifierOneVsOne::createSubClassifiers(void)
{
	// Clear any previous ones
	std::map< std::pair<uint32, uint32>, IAlgorithmProxy* >::iterator it;
	for(it = m_oSubClassifiers.begin(); it != m_oSubClassifiers.end(); it++)
	{
		IAlgorithmProxy* l_pSubClassifier = (*it).second;
		l_pSubClassifier->uninitialize();
		this->getAlgorithmManager().releaseAlgorithm(*l_pSubClassifier);
	}
	this->m_oSubClassifiers.clear();

	//Now let's instantiate all the sub classifiers
	for(size_t l_iFirstClass=0 ; l_iFirstClass < m_ui32NumberOfClasses; ++l_iFirstClass)
	{
		for(size_t l_iSecondClass = l_iFirstClass+1 ; l_iSecondClass < m_ui32NumberOfClasses ; ++l_iSecondClass)
		{
			const CIdentifier l_oSubClassifierAlgorithm = this->getAlgorithmManager().createAlgorithm(this->m_oSubClassifierAlgorithmIdentifier);
			if(l_oSubClassifierAlgorithm == OV_UndefinedIdentifier)
			{
				this->getLogManager() << LogLevel_Error << "Unable to instantiate classifier for class " << this->m_oSubClassifierAlgorithmIdentifier << "\n. Is the classifier still available in OpenViBE?";
				return false;
			}
			IAlgorithmProxy* l_pSubClassifier = &this->getAlgorithmManager().getAlgorithm(l_oSubClassifierAlgorithm);
			l_pSubClassifier->initialize();

			TParameterHandler < uint64 > ip_pNumberOfClasses(l_pSubClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_NumberOfClasses));
			ip_pNumberOfClasses = 2;

			//Set a references to the extra parameters input of the pairing strategy
			TParameterHandler< std::map<CString, CString>* > ip_pExtraParameters(l_pSubClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_ExtraParameter));
			ip_pExtraParameters.setReferenceTarget(this->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_ExtraParameter));

			m_oSubClassifiers[std::pair<uint32,uint32>(l_iFirstClass, l_iSecondClass)] = l_pSubClassifier;
		}
	}
	return true;
}

boolean CAlgorithmClassifierOneVsOne::designArchitecture(const OpenViBE::CIdentifier& rId, OpenViBE::uint32 rClassCount)
{
	if(!setSubClassifierIdentifier(rId))
	{
		return false;
	}

	m_ui32NumberOfClasses = rClassCount;


	return true;
}

XML::IXMLNode* CAlgorithmClassifierOneVsOne::getClassifierConfiguration(float64 f64FirstClass, float64 f64SecondClass, IAlgorithmProxy* pSubClassifier)
{
	XML::IXMLNode * l_pRes = XML::createNode(c_sSubClassifierNodeName);

	std::stringstream l_sFirstClass, l_sSecondClass;
	l_sFirstClass << f64FirstClass;
	l_sSecondClass << f64SecondClass;
	l_pRes->addAttribute(c_sFirstClassAtrributeName, l_sFirstClass.str().c_str());
	l_pRes->addAttribute(c_sSecondClassAttributeName, l_sSecondClass.str().c_str());

	TParameterHandler < XML::IXMLNode* > op_pConfiguration(pSubClassifier->getOutputParameter(OVTK_Algorithm_Classifier_OutputParameterId_Configuration));
	pSubClassifier->process(OVTK_Algorithm_Classifier_InputTriggerId_SaveConfiguration);
	l_pRes->addChild((XML::IXMLNode*)op_pConfiguration);

	return l_pRes;
}

XML::IXMLNode* CAlgorithmClassifierOneVsOne::getPairwiseDecisionConfiguration()
{
	if(!m_pDecisionStrategyAlgorithm) {
		return NULL;
	}

	XML::IXMLNode *l_pTempNode = XML::createNode(c_sPairwiseDecisionName);

	TParameterHandler < XML::IXMLNode* > op_pConfiguration(m_pDecisionStrategyAlgorithm->getOutputParameter(OVP_Algorithm_Classifier_Pairwise_OutputParameterId_Configuration));
	m_pDecisionStrategyAlgorithm->process(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_SaveConfiguration);
	l_pTempNode->addChild((XML::IXMLNode*)op_pConfiguration);

	l_pTempNode->addAttribute(c_sAlgorithmIdAttribute, m_oPairwiseDecisionIdentifier.toString());

	return l_pTempNode;
}

XML::IXMLNode* CAlgorithmClassifierOneVsOne::saveConfiguration(void)
{
	std::stringstream l_sClassifierCount;
	l_sClassifierCount << m_ui32NumberOfSubClassifiers;

	XML::IXMLNode *l_pOneVsOneNode = XML::createNode(c_sTypeNodeName);

	XML::IXMLNode *l_pTempNode = XML::createNode(c_sSubClassifierIdentifierNodeName);
	l_pTempNode->addAttribute(c_sAlgorithmIdAttribute,this->m_oSubClassifierAlgorithmIdentifier.toString());
	l_pTempNode->setPCData(this->getTypeManager().getEnumerationEntryNameFromValue(OVTK_TypeId_ClassificationAlgorithm, m_oSubClassifierAlgorithmIdentifier.toUInteger()).toASCIIString());
	l_pOneVsOneNode->addChild(l_pTempNode);

	l_pTempNode = XML::createNode(c_sSubClassifierCountNodeName);
	l_pTempNode->setPCData(l_sClassifierCount.str().c_str());
	l_pOneVsOneNode->addChild(l_pTempNode);

	l_pOneVsOneNode->addChild(this->getPairwiseDecisionConfiguration());

	XML::IXMLNode *l_pSubClassifersNode = XML::createNode(c_sSubClassifiersNodeName);

	std::map< std::pair<uint32, uint32>, IAlgorithmProxy* >::iterator it;
	for(it = m_oSubClassifiers.begin(); it != m_oSubClassifiers.end(); it++)
	{
		l_pSubClassifersNode->addChild(getClassifierConfiguration((*it).first.first, (*it).first.second, (*it).second));
	}
	l_pOneVsOneNode->addChild(l_pSubClassifersNode);

	return l_pOneVsOneNode;
}

boolean CAlgorithmClassifierOneVsOne::loadConfiguration(XML::IXMLNode *pConfigurationNode)
{
	XML::IXMLNode *l_pTempNode = pConfigurationNode->getChildByName(c_sSubClassifierIdentifierNodeName);

	CIdentifier l_pAlgorithmIdentifier;
	l_pAlgorithmIdentifier.fromString(l_pTempNode->getAttribute(c_sAlgorithmIdAttribute));

	if(!this->setSubClassifierIdentifier(l_pAlgorithmIdentifier)){
		//if the sub classifier doesn't have comparison function it is an error
		return false;
	}

	l_pTempNode = pConfigurationNode->getChildByName(c_sPairwiseDecisionName);
	CIdentifier l_pPairwiseIdentifier;
	l_pPairwiseIdentifier.fromString(l_pTempNode->getAttribute(c_sAlgorithmIdAttribute));
	if(l_pPairwiseIdentifier != m_oPairwiseDecisionIdentifier)
	{
		if(m_pDecisionStrategyAlgorithm != NULL){
			m_pDecisionStrategyAlgorithm->uninitialize();
			this->getAlgorithmManager().releaseAlgorithm(*m_pDecisionStrategyAlgorithm);
			m_pDecisionStrategyAlgorithm = NULL;
		}
		m_oPairwiseDecisionIdentifier = l_pPairwiseIdentifier;
		m_pDecisionStrategyAlgorithm = &this->getAlgorithmManager().getAlgorithm(this->getAlgorithmManager().createAlgorithm(m_oPairwiseDecisionIdentifier));
		m_pDecisionStrategyAlgorithm->initialize();
	}
	TParameterHandler < XML::IXMLNode* > ip_pConfiguration(m_pDecisionStrategyAlgorithm->getInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameterId_Configuration));
	ip_pConfiguration = l_pTempNode->getChild(0);

	TParameterHandler < CIdentifier *> ip_pClassificationAlgorithm(m_pDecisionStrategyAlgorithm->getInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameterId_AlgorithmIdentifier));
	ip_pClassificationAlgorithm = &l_pAlgorithmIdentifier;

	l_pTempNode = pConfigurationNode->getChildByName(c_sSubClassifierCountNodeName);
	std::stringstream l_sCountData(l_pTempNode->getPCData());
	l_sCountData >> m_ui32NumberOfSubClassifiers;

	// Invert the class count from subCls = numClass*(numClass-1)/2.
	const uint32 l_ui32DeltaCarre = 1+8*m_ui32NumberOfSubClassifiers;
	m_ui32NumberOfClasses = static_cast<uint32>((1+::sqrt(static_cast<double>(l_ui32DeltaCarre)))/2);

	TParameterHandler<uint64> ip_pClassCount(m_pDecisionStrategyAlgorithm->getInputParameter(OVP_Algorithm_Classifier_Pairwise_InputParameter_ClassCount));
	ip_pClassCount = m_ui32NumberOfClasses;

	if(!m_pDecisionStrategyAlgorithm->process(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_LoadConfiguration))
	{
		this->getLogManager() << LogLevel_Error << "Loading decision strategy configuration failed\n";
		return false;
	}
	if(!m_pDecisionStrategyAlgorithm->process(OVP_Algorithm_Classifier_Pairwise_InputTriggerId_Parameterize))
	{
		this->getLogManager() << LogLevel_Error << "Parameterizing decision strategy failed\n";
		return false;
	}

	return loadSubClassifierConfiguration(pConfigurationNode->getChildByName(c_sSubClassifiersNodeName));
}

uint32 CAlgorithmClassifierOneVsOne::getOutputProbabilityVectorLength()
{
	return static_cast<uint32>(m_ui32NumberOfClasses);
}

uint32 CAlgorithmClassifierOneVsOne::getOutputDistanceVectorLength()
{
	return 0;
}

boolean CAlgorithmClassifierOneVsOne::loadSubClassifierConfiguration(XML::IXMLNode *pSubClassifiersNode)
{
	createSubClassifiers();

	for(size_t i = 0; i < pSubClassifiersNode->getChildCount() ; ++i)
	{
		float64 l_f64FirstClass, l_f64SecondClass;

		//Now we have to restore class indexes
		XML::IXMLNode *l_pSubClassifierNode = pSubClassifiersNode->getChild(i);
		std::stringstream l_sFirstClass(l_pSubClassifierNode->getAttribute(c_sFirstClassAtrributeName));
		l_sFirstClass >> l_f64FirstClass;
		std::stringstream l_sSecondClass(l_pSubClassifierNode->getAttribute(c_sSecondClassAttributeName));
		l_sSecondClass >> l_f64SecondClass;

		IAlgorithmProxy* l_pSubClassifier = m_oSubClassifiers[std::pair<uint32,uint32>((uint32)l_f64FirstClass,(uint32)l_f64SecondClass)];

		TParameterHandler < XML::IXMLNode* > ip_pConfiguration(l_pSubClassifier->getInputParameter(OVTK_Algorithm_Classifier_InputParameterId_Configuration));
		ip_pConfiguration = l_pSubClassifierNode->getChild(0);
		if(!l_pSubClassifier->process(OVTK_Algorithm_Classifier_InputTriggerId_LoadConfiguration))
		{
			this->getLogManager() << LogLevel_Error << "Unable to load the configuration for the sub-classifier " << static_cast<uint64>(i+1) << "\n";
			return false;
		}
	}
	if(m_oSubClassifiers.size() != m_ui32NumberOfSubClassifiers)
	{
		this->getLogManager() << LogLevel_Error << "Number of loaded classifiers doesn't match the expected number\n";
		return false;
	}
	return true;
}

boolean CAlgorithmClassifierOneVsOne::setSubClassifierIdentifier(const OpenViBE::CIdentifier &rId)
{
	m_oSubClassifierAlgorithmIdentifier = rId;
	m_fAlgorithmComparison = getClassificationComparisonFunction(rId);

	if(m_fAlgorithmComparison == NULL)
	{
		this->getLogManager() << LogLevel_Error << "Cannot find the comparison function for the sub classifier\n";
		return false;
	}
	return true;
}
