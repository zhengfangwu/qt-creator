/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "googletest.h"
#include "testclangtool.h"

#include <sourcerangeextractor.h>
#include <sourcerangescontainer.h>

#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <clang/Basic/SourceManager.h>
#include <clang/Lex/Lexer.h>

#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

using testing::Contains;
using ::testing::Eq;
using ::testing::StrEq;

using ClangBackEnd::SourceRangeWithTextContainer;
using ClangBackEnd::SourceRangeExtractor;

namespace {

class SourceRangeExtractor : public ::testing::Test
{
protected:
    void SetUp() override;
    void TearDown() override;

protected:
    TestClangTool clangTool{TESTDATA_DIR, "sourcerangeextractor_location.cpp", "",  {"cc", "sourcerangeextractor_location.cpp"}};
    ClangBackEnd::SourceRangesContainer sourceRangesContainer;
    const clang::SourceManager &sourceManager{clangTool.sourceManager()};
    ClangBackEnd::SourceRangeExtractor extractor{sourceManager, clangTool.languageOptions(), sourceRangesContainer};
    clang::SourceLocation startLocation = sourceManager.getLocForStartOfFile(sourceManager.getMainFileID());
    clang::SourceLocation endLocation = sourceManager.getLocForStartOfFile(sourceManager.getMainFileID()).getLocWithOffset(4);
    clang::SourceRange sourceRange{startLocation, endLocation};
    clang::SourceRange extendedSourceRange{startLocation, endLocation.getLocWithOffset(5)};
};

TEST_F(SourceRangeExtractor, ExtractSourceRangeContainer)
{
    SourceRangeWithTextContainer sourceRangeContainer{1, 1, 1, 0, 1, 10, 9, Utils::SmallString("int value;")};

    extractor.addSourceRange(sourceRange);

    ASSERT_THAT(extractor.sourceRangeWithTextContainers(), Contains(sourceRangeContainer));
}

TEST_F(SourceRangeExtractor, ExtendedSourceRange)
{
    auto range = extractor.extendSourceRangeToLastTokenEnd(sourceRange);

    ASSERT_THAT(range, extendedSourceRange);
}

TEST_F(SourceRangeExtractor, FindStartOfLineInEmptyBuffer)
{
    clang::StringRef text = "";

    auto found = ::SourceRangeExtractor::findStartOfLineInBuffer(text, 0);

    ASSERT_THAT(found, StrEq(""));
}

TEST_F(SourceRangeExtractor, FindStartOfLineInBufferInFirstLine)
{
    clang::StringRef text = "first line";

    auto found = ::SourceRangeExtractor::findStartOfLineInBuffer(text, 5);

    ASSERT_THAT(found, StrEq("first line"));
}

TEST_F(SourceRangeExtractor, FindStartOfNewLineInBufferInSecondLine)
{
    clang::StringRef text = "first line\nsecond line";

    auto found = ::SourceRangeExtractor::findStartOfLineInBuffer(text, 15);

    ASSERT_THAT(found, StrEq("second line"));
}

TEST_F(SourceRangeExtractor, FindStartOfCarriageReturnInBufferInSecondLine)
{
    clang::StringRef text = "first line\rsecond line";

    auto found = ::SourceRangeExtractor::findStartOfLineInBuffer(text, 15);

    ASSERT_THAT(found, StrEq("second line"));
}

TEST_F(SourceRangeExtractor, FindStartOfNewLineCarriageReturnInBufferInSecondLine)
{
    clang::StringRef text = "first line\n\rsecond line";

    auto found = ::SourceRangeExtractor::findStartOfLineInBuffer(text, 15);

    ASSERT_THAT(found, StrEq("second line"));
}

TEST_F(SourceRangeExtractor, FindEndOfLineInEmptyBuffer)
{
    clang::StringRef text = "";

    auto found = ::SourceRangeExtractor::findEndOfLineInBuffer(text, 0);

    ASSERT_THAT(found, StrEq(""));
}

TEST_F(SourceRangeExtractor, FindEndOfLineInBuffer)
{
    clang::StringRef text = "first line";

    auto found = ::SourceRangeExtractor::findEndOfLineInBuffer(text, 5);

    ASSERT_THAT(found, StrEq(""));
}

TEST_F(SourceRangeExtractor, FindEndOfLineInBufferInFirstLineWithNewLine)
{
    clang::StringRef text = "first line\nsecond line\nthird line";

    auto found = ::SourceRangeExtractor::findEndOfLineInBuffer(text, 15);

    ASSERT_THAT(found, StrEq("\nthird line"));
}

TEST_F(SourceRangeExtractor, FindEndOfLineInBufferInFirstLineWithCarriageReturn)
{
    clang::StringRef text = "first line\rsecond line\rthird line";

    auto found = ::SourceRangeExtractor::findEndOfLineInBuffer(text, 15);

    ASSERT_THAT(found, StrEq("\rthird line"));
}

TEST_F(SourceRangeExtractor, EpandText)
{
    clang::StringRef text = "first line\nsecond line\nthird line\nforth line";

    auto expandedText = ::SourceRangeExtractor::getExpandedText(text, 15, 25);

    ASSERT_THAT(expandedText, StrEq("second line\nthird line"));
}

void SourceRangeExtractor::SetUp()
{
    TestGlobal::setSourceManager(&sourceManager);
}

void SourceRangeExtractor::TearDown()
{
    TestGlobal::setSourceManager(nullptr);
}
}

