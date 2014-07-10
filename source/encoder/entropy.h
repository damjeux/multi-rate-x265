/*****************************************************************************
* Copyright (C) 2013 x265 project
*
* Authors: Steve Borho <steve@borho.org>
*
* This program is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02111, USA.
*
* This program is also available under a commercial proprietary license.
* For more information, contact us at license @ x265.com.
*****************************************************************************/

#ifndef X265_ENTROPY_H
#define X265_ENTROPY_H

#include "common.h"
#include "bitstream.h"
#include "frame.h"

#include "TLibCommon/TComSlice.h"
#include "TLibCommon/TComDataCU.h"
#include "TLibCommon/TComTrQuant.h"
#include "TLibCommon/TComSampleAdaptiveOffset.h"

namespace x265 {
// private namespace

class TComScalingList;

enum SplitType
{
    DONT_SPLIT            = 0,
    VERTICAL_SPLIT        = 1,
    QUAD_SPLIT            = 2,
    NUMBER_OF_SPLIT_MODES = 3
};

struct TURecurse
{
    uint32_t section;
    uint32_t splitMode;
    uint32_t absPartIdxTURelCU;
    uint32_t absPartIdxStep;

    void initSection(SplitType splitType, uint32_t _absPartIdxStep, uint32_t _absPartIdxTU = 0)
    {
        static const uint32_t partIdxStepShift[NUMBER_OF_SPLIT_MODES] = { 0, 1, 2 };
        section           = 0;
        absPartIdxTURelCU = _absPartIdxTU;
        splitMode         = (uint32_t)splitType;
        absPartIdxStep    = _absPartIdxStep >> partIdxStepShift[splitMode];
    }

    bool isNextSection()
    {
        if (splitMode == DONT_SPLIT)
        {
            section++;
            return false;
        }
        else
        {
            absPartIdxTURelCU += absPartIdxStep;

            section++;
            return section < (uint32_t)(1 << splitMode);
        }
    }

    bool isLastSection() const
    {
        return (section + 1) >= (uint32_t)(1 << splitMode);
    }
};

class CABAC
{
public:

    BitInterface* m_bitIf;
    uint32_t   m_low;
    uint32_t   m_range;
    uint32_t   m_bufferedByte;
    int        m_numBufferedBytes;
    int        m_bitsLeft;
    uint64_t   m_fracBits;
    bool       m_bIsCounter;

    CABAC();

    void  init(BitInterface* bitIf);

    void  start();
    void  finish();
    void  copyState(CABAC& binIf);
    void  flush();

    void  resetBac();
    void  resetBits();

    uint32_t getNumWrittenBits()
    {
        X265_CHECK(m_bIsCounter && !m_bitIf->getNumberOfWrittenBits(), "counter mode expected\n");
        return uint32_t(m_fracBits >> 15);
    }

    void  encodeBin(uint32_t binValue, ContextModel& ctxModel);
    void  encodeBinEP(uint32_t binValue);
    void  encodeBinsEP(uint32_t binValues, int numBins);
    void  encodeBinTrm(uint32_t binValue);

protected:

    void writeOut();

};

class SBac : public SyntaxElementWriter
{
public:

    uint64_t      m_pad;
    ContextModel  m_contextModels[MAX_OFF_CTX_MOD];
    CABAC         m_cabac;

    SBac()                            { memset(m_contextModels, 0, sizeof(m_contextModels)); }

    CABAC* getEncBinIf()              { return &m_cabac; }
    void  zeroFract()                 { m_cabac.m_fracBits = 0;  }
    void  resetBits()                 { m_cabac.resetBits(); m_bitIf->resetBits(); }

    uint32_t getNumberOfWrittenBits() { return m_cabac.getNumWrittenBits(); }

    void resetEntropy(TComSlice *slice);
    void determineCabacInitIdx(TComSlice *slice);
    void setBitstream(BitInterface* p);

    // SBAC RD
    void load(SBac& src);
    void loadIntraDirModeLuma(SBac& src);
    void store(SBac& dest);
    void loadContexts(SBac& src);

    void codeVPS(TComVPS* vps);
    void codeSPS(TComSPS* sps, TComScalingList *scalingList);
    void codePPS(TComPPS* pps, TComScalingList *scalingList);
    void codeVUI(TComVUI* vui, TComSPS* sps);
    void codeAUD(TComSlice *slice);
    void codePTL(TComPTL* ptl, bool profilePresentFlag, int maxNumSubLayersMinus1);
    void codeProfileTier(ProfileTierLevel* ptl);

    void codeSliceHeader(TComSlice* slice);
    void codeTilesWPPEntryPoint(TComSlice* slice);
    void codeShortTermRefPicSet(TComReferencePictureSet* rps, bool calledFromSliceHeader, int idx);
    void codeSliceFinish();

    void codeHrdParameters(TComHRD* hrd, bool commonInfPresentFlag, uint32_t maxNumSubLayersMinus1);

    void codeSaoMaxUvlc(uint32_t code, uint32_t maxSymbol);
    void codeSaoMerge(uint32_t code);
    void codeSaoTypeIdx(uint32_t code);
    void codeSaoUflc(uint32_t length, uint32_t code);
    void codeSAOSign(uint32_t code) { m_cabac.encodeBinEP(code); }
    void codeSaoOffset(SaoLcuParam* saoLcuParam, uint32_t compIdx);
    void codeSaoUnitInterleaving(int compIdx, bool saoFlag, int rx, int ry, SaoLcuParam* saoLcuParam, int cuAddrInSlice, int cuAddrUpInSlice, int allowMergeLeft, int allowMergeUp);

    void codeTerminatingBit(uint32_t lsLast);
    void codeScalingList(TComScalingList*);

    void codeCUTransquantBypassFlag(TComDataCU* cu, uint32_t absPartIdx);
    void codeSkipFlag(TComDataCU* cu, uint32_t absPartIdx);
    void codeMergeFlag(TComDataCU* cu, uint32_t absPartIdx);
    void codeMergeIndex(TComDataCU* cu, uint32_t absPartIdx);
    void codeSplitFlag(TComDataCU* cu, uint32_t absPartIdx, uint32_t depth);
    void codeRefFrmIdx(TComDataCU* cu, uint32_t absPartIdx, int list);
    void codeMVPIdx(uint32_t symbol);
    void codeMvd(TComDataCU* cu, uint32_t absPartIdx, int list);

    void codePartSize(TComDataCU* cu, uint32_t absPartIdx, uint32_t depth);
    void codePredMode(TComDataCU* cu, uint32_t absPartIdx);
    void codeTransformSubdivFlag(uint32_t symbol, uint32_t ctx);
    void codeQtCbf(TComDataCU* cu, uint32_t absPartIdx, uint32_t absPartIdxStep, uint32_t width, uint32_t height, TextType ttype, uint32_t trDepth, bool lowestLevel);
    void codeQtCbf(TComDataCU* cu, uint32_t absPartIdx, TextType ttype, uint32_t trDepth);
    void codeQtRootCbf(TComDataCU* cu, uint32_t absPartIdx);
    void codeQtCbfZero(TComDataCU* cu, TextType ttype, uint32_t trDepth);
    void codeQtRootCbfZero(TComDataCU* cu);

    void codePUWise(TComDataCU* cu, uint32_t absPartIdx);
    void codeRefFrmIdxPU(TComDataCU* subCU, uint32_t absPartIdx, int list);
    void codePredInfo(TComDataCU* cu, uint32_t absPartIdx);
    void codeCoeff(TComDataCU* cu, uint32_t absPartIdx, uint32_t depth, uint32_t cuSize, bool& bCodeDQP);

    void codeIntraDirLumaAng(TComDataCU* cu, uint32_t absPartIdx, bool isMultiple);
    void codeIntraDirChroma(TComDataCU* cu, uint32_t absPartIdx);
    void codeInterDir(TComDataCU* cu, uint32_t absPartIdx);

    void codeDeltaQP(TComDataCU* cu, uint32_t absPartIdx);
    void codeLastSignificantXY(uint32_t posx, uint32_t posy, uint32_t log2TrSize, TextType ttype, uint32_t scanIdx);
    void codeCoeffNxN(TComDataCU* cu, coeff_t* coef, uint32_t absPartIdx, uint32_t log2TrSize, TextType ttype);
    void codeTransformSkipFlags(TComDataCU* cu, uint32_t absPartIdx, uint32_t trSize, TextType ttype);

    // RDO functions
    void estBit(estBitsSbacStruct* estBitsSbac, int trSize, TextType ttype);
    void estCBFBit(estBitsSbacStruct* estBitsSbac);
    void estSignificantCoeffGroupMapBit(estBitsSbacStruct* estBitsSbac, TextType ttype);
    void estSignificantMapBit(estBitsSbacStruct* estBitsSbac, int trSize, TextType ttype);
    void estSignificantCoefficientsBit(estBitsSbacStruct* estBitsSbac, TextType ttype);

private:

    void writeUnaryMaxSymbol(uint32_t symbol, ContextModel* scmModel, int offset, uint32_t maxSymbol);
    void writeEpExGolomb(uint32_t symbol, uint32_t count);
    void writeCoefRemainExGolomb(uint32_t symbol, const uint32_t absGoRice);

    bool findMatchingLTRP(TComSlice* slice, uint32_t *ltrpsIndex, int ltrpPOC, bool usedFlag);
    void codePredWeightTable(TComSlice* slice);

    struct CoeffCodeState
    {
        uint32_t  bakAbsPartIdx;
        uint32_t  bakChromaOffset;
        uint32_t  bakAbsPartIdxCU;
    };

    void encodeTransform(TComDataCU* cu, CoeffCodeState& state, uint32_t offsetLumaOffset, uint32_t offsetChroma, uint32_t absPartIdx, uint32_t absPartIdxStep, uint32_t depth, uint32_t tuSize, uint32_t uiTrIdx, bool& bCodeDQP);

    void copyFrom(SBac& src);
    void copyContextsFrom(SBac& src);
    void codeScalingList(TComScalingList* scalingList, uint32_t sizeId, uint32_t listId);
};
}

#endif // ifndef X265_ENTROPY_H
