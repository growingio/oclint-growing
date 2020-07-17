#include "oclint/AbstractASTVisitorRule.h"
#include "oclint/RuleSet.h"
#include "oclint/RuleConfiguration.h"
using namespace std;
using namespace clang;
using namespace oclint;

class GrowingIOCodeCheckerRule : public AbstractASTVisitorRule<GrowingIOCodeCheckerRule>
{
public:
    virtual const string name() const override
    {
        return "GrowingIOCodeChecker";
    }

    virtual int priority() const override
    {
        return 2;
    }

    virtual const string category() const override
    {
        return "custom";
    }

#ifdef DOCGEN
    virtual const std::string since() const override
    {
        return "0.15";
    }

    virtual const std::string description() const override
    {
        return ""; // TODO: fill in the description of the rule.
    }

    virtual const std::string example() const override
    {
        return R"rst(
.. code-block:: cpp

    void example()
    {
        // TODO: modify the example for this rule.
    }
        )rst";
    }

    /* fill in the file name only when it does not match the rule identifier
    virtual const std::string fileName() const override
    {
        return "";
    }
    */

    /* add each threshold's key, description, and its default value to the map.
    virtual const std::map<std::string, std::string> thresholds() const override
    {
        std::map<std::string, std::string> thresholdMapping;
        return thresholdMapping;
    }
    */

    /* add additional document for the rule, like references, notes, etc.
    virtual const std::string additionalDocument() const override
    {
        return "";
    }
    */

    /* uncomment this method when the rule is enabled to be suppressed.
    virtual bool enableSuppress() const override
    {
        return true;
    }
    */
#endif

    virtual void setUp() override {}
    virtual void tearDown() override {}

    
    /* Visit Stmt
    bool VisitStmt(Stmt *node)
    {
        return true;
    }
     */

    /* Visit NullStmt
    bool VisitNullStmt(NullStmt *node)
    {
        return true;
    }
     */

    /* Visit CompoundStmt
    bool VisitCompoundStmt(CompoundStmt *node)
    {
        return true;
    }
     */

    bool VisitIfStmt(IfStmt *node)
    {
        //获得操作符号
        Expr* condExpr = node->getCond();
        if (condExpr) {
            BinaryOperator *binaryOperator = dyn_cast_or_null<BinaryOperator>(condExpr);
            if (binaryOperator) {
                ///如果是 == 和 !=
                if ((binaryOperator && binaryOperator->getOpcode() == BO_NE) || (binaryOperator && binaryOperator->getOpcode() == BO_EQ)) {
                    //获得左右两侧 expr
                    ImplicitCastExpr *lhs = dyn_cast_or_null<ImplicitCastExpr>(binaryOperator->getLHS());
                    ImplicitCastExpr *rhs = dyn_cast_or_null<ImplicitCastExpr>(binaryOperator->getRHS());
                    //当expr为BOOL类型时，强转ObjCBoolLiteralExpr并判断是否为true，然后加入violation
                    if (GrowingCheckImplicitCastExpr(lhs) || GrowingCheckImplicitCastExpr(rhs)) {
                        SourceLocation loc = node->getIfLoc();
                        string message = "if语句中，不能与YES的BOOL值进行直接比较";
                        addViolation(loc, loc, this, message);
                    }
                }

                //操作符左右两侧空格判断
//                GrowingSpaceViolation(binaryOperator->getOperatorLoc().getLocWithOffset(-1),"运算符\'" + binaryOperator->getOpcodeStr().str() + "\'两侧需要添加空格",0);
//                //实际获取的size大小要比实际大1，这里减1
//                //                printf("size %zu\n",binaryOperator->getOpcodeStr().size());
//                GrowingSpaceViolation(binaryOperator->getOperatorLoc().getLocWithOffset(binaryOperator->getOpcodeStr().size()),"运算符\'" + binaryOperator->getOpcodeStr().str() + "\'两侧需要添加空格",0);


            }

        }
//        //判断if后跟空格 if从i开始,位移2位
        GrowingSpaceViolation(node->getIfLoc(),"\'if\'之后要添加空格",2);
        
        //判断大括号
        clang::SourceManager *sourceManager = &_carrier->getSourceManager();
        clang::SourceLocation startFileLoc = sourceManager->getFileLoc(node->getIfLoc());
        int beginLine = sourceManager->getPresumedLineNumber(startFileLoc);
        //获得if执行语句
        clang::CompoundStmt *compoundStmt = clang::dyn_cast_or_null<clang::CompoundStmt>(node->getThen());
        if (compoundStmt) {
            //1.判断{是否与if同行
            int compoundLine = sourceManager->getPresumedLineNumber(compoundStmt->getLBracLoc());
            //如果if语句和左侧大括号语句同一行
            if (beginLine == compoundLine) {
                SourceLocation loc = compoundStmt->getLBracLoc().getLocWithOffset(-1);
                const char *str = sourceManager->getCharacterData(loc);
                if (str[0] != ' ') {
                    SourceLocation loc = compoundStmt->getLBracLoc();
                    string message = "if语句中，\'{\'前需要添加空格";
                    addViolation(loc, loc, this, message);
                }
            }else {
//                SourceLocation loc = compoundStmt->getLBracLoc();
//                string message = "if语句中，\'{\'需要和if同行";
//                addViolation(loc, loc, this, message);
            }
//            //2.判断}后是否跟空格或者'\n'
            GrowingSpaceOrNewlineViolation(compoundStmt->getRBracLoc(), "\'}\'后需要添加空格", 1);
        }
        
        
        //有else的if语句必须被大括号包裹
        if (node->hasElseStorage()) {
            //if 的then必须要是CompoundStmt
            
            if (!compoundStmt) {
                SourceLocation loc = node->getThen()->getBeginLoc();
                string message = "\'if...else\'语句必须使用大括号";
                addViolation(loc, loc, this, message);
            }
            Stmt* elstst = node->getElse();
            clang::IfStmt *ifStmt = clang::dyn_cast_or_null<clang::IfStmt>(elstst);
            if (!ifStmt) {
                if (node->getElse()->getStmtClass() != clang::AbstractConditionalOperator::CompoundStmtClass) {
                    SourceLocation loc = elstst->getEndLoc();
                    string message = "\'if...else\'语句必须使用大括号";
                    addViolation(loc, loc, this, message);
                }
                //                printf("not a if stmt : %s\n",node->getElse()->getStmtClassName());
            }
        }
        
        
        return true;
    }
        
    void GrowingSpaceOrNewlineViolation(SourceLocation sourceLoc,string message,int offset) {
        clang::SourceManager *sourceManager = &_carrier->getSourceManager();
        SourceLocation loc = sourceLoc.getLocWithOffset(offset);
        const char *str = sourceManager->getCharacterData(loc);
        if (str[0] != ' ' || str[0] != '\n' || str[0] != ')') {
            //                printf("char is %c\n",str[0]);
        }else {
            addViolation(sourceLoc, sourceLoc, this, message);
        }
    }
    
    void GrowingSpaceViolation(SourceLocation sourceLoc,string message,int offset) {
        
        clang::SourceManager *sourceManager = &_carrier->getSourceManager();
        //        printf("%s\n",sourceLoc.printToString(*sourceManager).c_str());
        SourceLocation loc = sourceLoc.getLocWithOffset(offset);
        const char *str = sourceManager->getCharacterData(loc);
        if (str[0] != ' ') {
            //            printf("char is %c\n",str[0]);
            addViolation(sourceLoc, sourceLoc, this, message);
        }
    }
    
    bool GrowingCheckImplicitCastExpr(ImplicitCastExpr *expr) {
        if (!expr) return false;
        Expr *subExpr = expr->getSubExpr();
        if (!subExpr) return false;
        string operatorStr = subExpr->getType().getAsString();
        if (operatorStr == "BOOL") {
            ObjCBoolLiteralExpr *trueObjCBOOL =
            dyn_cast_or_null<ObjCBoolLiteralExpr>(subExpr);
            if (trueObjCBOOL && trueObjCBOOL->getValue()) {
                return true;
            }
        }
        return false;
    }

    /* Visit SwitchStmt
    bool VisitSwitchStmt(SwitchStmt *node)
    {
        return true;
    }
     */

    /* Visit WhileStmt
    bool VisitWhileStmt(WhileStmt *node)
    {
        return true;
    }
     */

    /* Visit DoStmt
    bool VisitDoStmt(DoStmt *node)
    {
        return true;
    }
     */

    /* Visit ForStmt
    bool VisitForStmt(ForStmt *node)
    {
        return true;
    }
     */

    /* Visit GotoStmt
    bool VisitGotoStmt(GotoStmt *node)
    {
        return true;
    }
     */

    /* Visit IndirectGotoStmt
    bool VisitIndirectGotoStmt(IndirectGotoStmt *node)
    {
        return true;
    }
     */

    /* Visit ContinueStmt
    bool VisitContinueStmt(ContinueStmt *node)
    {
        return true;
    }
     */

    /* Visit BreakStmt
    bool VisitBreakStmt(BreakStmt *node)
    {
        return true;
    }
     */

    /* Visit ReturnStmt
    bool VisitReturnStmt(ReturnStmt *node)
    {
        return true;
    }
     */

    /* Visit DeclStmt 
    bool VisitDeclStmt (DeclStmt  *node)
    {
        return true;
    }
     */

    /* Visit SwitchCase
    bool VisitSwitchCase(SwitchCase *node)
    {
        return true;
    }
     */

    /* Visit CaseStmt
    bool VisitCaseStmt(CaseStmt *node)
    {
        return true;
    }
     */

    /* Visit DefaultStmt
    bool VisitDefaultStmt(DefaultStmt *node)
    {
        return true;
    }
     */

    /* Visit CapturedStmt
    bool VisitCapturedStmt(CapturedStmt *node)
    {
        return true;
    }
     */

    /* Visit ValueStmt
    bool VisitValueStmt(ValueStmt *node)
    {
        return true;
    }
     */

    /* Visit LabelStmt
    bool VisitLabelStmt(LabelStmt *node)
    {
        return true;
    }
     */

    /* Visit AttributedStmt
    bool VisitAttributedStmt(AttributedStmt *node)
    {
        return true;
    }
     */

    /* Visit AsmStmt
    bool VisitAsmStmt(AsmStmt *node)
    {
        return true;
    }
     */

    /* Visit GCCAsmStmt
    bool VisitGCCAsmStmt(GCCAsmStmt *node)
    {
        return true;
    }
     */

    /* Visit MSAsmStmt
    bool VisitMSAsmStmt(MSAsmStmt *node)
    {
        return true;
    }
     */

    /* Visit ObjCAtTryStmt
    bool VisitObjCAtTryStmt(ObjCAtTryStmt *node)
    {
        return true;
    }
     */

    /* Visit ObjCAtCatchStmt
    bool VisitObjCAtCatchStmt(ObjCAtCatchStmt *node)
    {
        return true;
    }
     */

    /* Visit ObjCAtFinallyStmt
    bool VisitObjCAtFinallyStmt(ObjCAtFinallyStmt *node)
    {
        return true;
    }
     */

    /* Visit ObjCAtThrowStmt
    bool VisitObjCAtThrowStmt(ObjCAtThrowStmt *node)
    {
        return true;
    }
     */

    /* Visit ObjCAtSynchronizedStmt
    bool VisitObjCAtSynchronizedStmt(ObjCAtSynchronizedStmt *node)
    {
        return true;
    }
     */

    /* Visit ObjCForCollectionStmt
    bool VisitObjCForCollectionStmt(ObjCForCollectionStmt *node)
    {
        return true;
    }
     */

    /* Visit ObjCAutoreleasePoolStmt
    bool VisitObjCAutoreleasePoolStmt(ObjCAutoreleasePoolStmt *node)
    {
        return true;
    }
     */

    /* Visit CXXCatchStmt
    bool VisitCXXCatchStmt(CXXCatchStmt *node)
    {
        return true;
    }
     */

    /* Visit CXXTryStmt
    bool VisitCXXTryStmt(CXXTryStmt *node)
    {
        return true;
    }
     */

    /* Visit CXXForRangeStmt
    bool VisitCXXForRangeStmt(CXXForRangeStmt *node)
    {
        return true;
    }
     */

    /* Visit CoroutineBodyStmt
    bool VisitCoroutineBodyStmt(CoroutineBodyStmt *node)
    {
        return true;
    }
     */

    /* Visit CoreturnStmt
    bool VisitCoreturnStmt(CoreturnStmt *node)
    {
        return true;
    }
     */

    /* Visit Expr
    bool VisitExpr(Expr *node)
    {
        return true;
    }
     */

    /* Visit PredefinedExpr
    bool VisitPredefinedExpr(PredefinedExpr *node)
    {
        return true;
    }
     */

    /* Visit DeclRefExpr
    bool VisitDeclRefExpr(DeclRefExpr *node)
    {
        return true;
    }
     */

    /* Visit IntegerLiteral
    bool VisitIntegerLiteral(IntegerLiteral *node)
    {
        return true;
    }
     */

    /* Visit FixedPointLiteral
    bool VisitFixedPointLiteral(FixedPointLiteral *node)
    {
        return true;
    }
     */

    /* Visit FloatingLiteral
    bool VisitFloatingLiteral(FloatingLiteral *node)
    {
        return true;
    }
     */

    /* Visit ImaginaryLiteral
    bool VisitImaginaryLiteral(ImaginaryLiteral *node)
    {
        return true;
    }
     */

    /* Visit StringLiteral
    bool VisitStringLiteral(StringLiteral *node)
    {
        return true;
    }
     */

    /* Visit CharacterLiteral
    bool VisitCharacterLiteral(CharacterLiteral *node)
    {
        return true;
    }
     */

    /* Visit ParenExpr
    bool VisitParenExpr(ParenExpr *node)
    {
        return true;
    }
     */

    /* Visit UnaryOperator
    bool VisitUnaryOperator(UnaryOperator *node)
    {
        return true;
    }
     */

    /* Visit OffsetOfExpr
    bool VisitOffsetOfExpr(OffsetOfExpr *node)
    {
        return true;
    }
     */

    /* Visit UnaryExprOrTypeTraitExpr
    bool VisitUnaryExprOrTypeTraitExpr(UnaryExprOrTypeTraitExpr *node)
    {
        return true;
    }
     */

    /* Visit ArraySubscriptExpr
    bool VisitArraySubscriptExpr(ArraySubscriptExpr *node)
    {
        return true;
    }
     */

    /* Visit OMPArraySectionExpr
    bool VisitOMPArraySectionExpr(OMPArraySectionExpr *node)
    {
        return true;
    }
     */

    /* Visit OMPIteratorExpr
    bool VisitOMPIteratorExpr(OMPIteratorExpr *node)
    {
        return true;
    }
     */

    /* Visit CallExpr
    bool VisitCallExpr(CallExpr *node)
    {
        return true;
    }
     */

    /* Visit MemberExpr
    bool VisitMemberExpr(MemberExpr *node)
    {
        return true;
    }
     */

    /* Visit CastExpr
    bool VisitCastExpr(CastExpr *node)
    {
        return true;
    }
     */

    /* Visit BinaryOperator
    bool VisitBinaryOperator(BinaryOperator *node)
    {
        return true;
    }
     */

    /* Visit CompoundAssignOperator
    bool VisitCompoundAssignOperator(CompoundAssignOperator *node)
    {
        return true;
    }
     */

    /* Visit AbstractConditionalOperator
    bool VisitAbstractConditionalOperator(AbstractConditionalOperator *node)
    {
        return true;
    }
     */

    /* Visit ConditionalOperator
    bool VisitConditionalOperator(ConditionalOperator *node)
    {
        return true;
    }
     */

    /* Visit BinaryConditionalOperator
    bool VisitBinaryConditionalOperator(BinaryConditionalOperator *node)
    {
        return true;
    }
     */

    /* Visit ImplicitCastExpr
    bool VisitImplicitCastExpr(ImplicitCastExpr *node)
    {
        return true;
    }
     */

    /* Visit ExplicitCastExpr
    bool VisitExplicitCastExpr(ExplicitCastExpr *node)
    {
        return true;
    }
     */

    /* Visit CStyleCastExpr
    bool VisitCStyleCastExpr(CStyleCastExpr *node)
    {
        return true;
    }
     */

    /* Visit OMPArrayShapingExpr
    bool VisitOMPArrayShapingExpr(OMPArrayShapingExpr *node)
    {
        return true;
    }
     */

    /* Visit CompoundLiteralExpr
    bool VisitCompoundLiteralExpr(CompoundLiteralExpr *node)
    {
        return true;
    }
     */

    /* Visit ExtVectorElementExpr
    bool VisitExtVectorElementExpr(ExtVectorElementExpr *node)
    {
        return true;
    }
     */

    /* Visit InitListExpr
    bool VisitInitListExpr(InitListExpr *node)
    {
        return true;
    }
     */

    /* Visit DesignatedInitExpr
    bool VisitDesignatedInitExpr(DesignatedInitExpr *node)
    {
        return true;
    }
     */

    /* Visit DesignatedInitUpdateExpr
    bool VisitDesignatedInitUpdateExpr(DesignatedInitUpdateExpr *node)
    {
        return true;
    }
     */

    /* Visit ImplicitValueInitExpr
    bool VisitImplicitValueInitExpr(ImplicitValueInitExpr *node)
    {
        return true;
    }
     */

    /* Visit NoInitExpr
    bool VisitNoInitExpr(NoInitExpr *node)
    {
        return true;
    }
     */

    /* Visit ArrayInitLoopExpr
    bool VisitArrayInitLoopExpr(ArrayInitLoopExpr *node)
    {
        return true;
    }
     */

    /* Visit ArrayInitIndexExpr
    bool VisitArrayInitIndexExpr(ArrayInitIndexExpr *node)
    {
        return true;
    }
     */

    /* Visit ParenListExpr
    bool VisitParenListExpr(ParenListExpr *node)
    {
        return true;
    }
     */

    /* Visit VAArgExpr
    bool VisitVAArgExpr(VAArgExpr *node)
    {
        return true;
    }
     */

    /* Visit GenericSelectionExpr
    bool VisitGenericSelectionExpr(GenericSelectionExpr *node)
    {
        return true;
    }
     */

    /* Visit PseudoObjectExpr
    bool VisitPseudoObjectExpr(PseudoObjectExpr *node)
    {
        return true;
    }
     */

    /* Visit SourceLocExpr
    bool VisitSourceLocExpr(SourceLocExpr *node)
    {
        return true;
    }
     */

    /* Visit FullExpr
    bool VisitFullExpr(FullExpr *node)
    {
        return true;
    }
     */

    /* Visit ConstantExpr
    bool VisitConstantExpr(ConstantExpr *node)
    {
        return true;
    }
     */

    /* Visit AtomicExpr
    bool VisitAtomicExpr(AtomicExpr *node)
    {
        return true;
    }
     */

    /* Visit AddrLabelExpr
    bool VisitAddrLabelExpr(AddrLabelExpr *node)
    {
        return true;
    }
     */

    /* Visit StmtExpr
    bool VisitStmtExpr(StmtExpr *node)
    {
        return true;
    }
     */

    /* Visit ChooseExpr
    bool VisitChooseExpr(ChooseExpr *node)
    {
        return true;
    }
     */

    /* Visit GNUNullExpr
    bool VisitGNUNullExpr(GNUNullExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXOperatorCallExpr
    bool VisitCXXOperatorCallExpr(CXXOperatorCallExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXMemberCallExpr
    bool VisitCXXMemberCallExpr(CXXMemberCallExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXRewrittenBinaryOperator
    bool VisitCXXRewrittenBinaryOperator(CXXRewrittenBinaryOperator *node)
    {
        return true;
    }
     */

    /* Visit CXXNamedCastExpr
    bool VisitCXXNamedCastExpr(CXXNamedCastExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXStaticCastExpr
    bool VisitCXXStaticCastExpr(CXXStaticCastExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXDynamicCastExpr
    bool VisitCXXDynamicCastExpr(CXXDynamicCastExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXReinterpretCastExpr
    bool VisitCXXReinterpretCastExpr(CXXReinterpretCastExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXConstCastExpr
    bool VisitCXXConstCastExpr(CXXConstCastExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXAddrspaceCastExpr
    bool VisitCXXAddrspaceCastExpr(CXXAddrspaceCastExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXFunctionalCastExpr
    bool VisitCXXFunctionalCastExpr(CXXFunctionalCastExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXTypeidExpr
    bool VisitCXXTypeidExpr(CXXTypeidExpr *node)
    {
        return true;
    }
     */

    /* Visit UserDefinedLiteral
    bool VisitUserDefinedLiteral(UserDefinedLiteral *node)
    {
        return true;
    }
     */

    /* Visit CXXBoolLiteralExpr
    bool VisitCXXBoolLiteralExpr(CXXBoolLiteralExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXNullPtrLiteralExpr
    bool VisitCXXNullPtrLiteralExpr(CXXNullPtrLiteralExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXThisExpr
    bool VisitCXXThisExpr(CXXThisExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXThrowExpr
    bool VisitCXXThrowExpr(CXXThrowExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXDefaultArgExpr
    bool VisitCXXDefaultArgExpr(CXXDefaultArgExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXDefaultInitExpr
    bool VisitCXXDefaultInitExpr(CXXDefaultInitExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXScalarValueInitExpr
    bool VisitCXXScalarValueInitExpr(CXXScalarValueInitExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXStdInitializerListExpr
    bool VisitCXXStdInitializerListExpr(CXXStdInitializerListExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXNewExpr
    bool VisitCXXNewExpr(CXXNewExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXDeleteExpr
    bool VisitCXXDeleteExpr(CXXDeleteExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXPseudoDestructorExpr
    bool VisitCXXPseudoDestructorExpr(CXXPseudoDestructorExpr *node)
    {
        return true;
    }
     */

    /* Visit TypeTraitExpr
    bool VisitTypeTraitExpr(TypeTraitExpr *node)
    {
        return true;
    }
     */

    /* Visit ArrayTypeTraitExpr
    bool VisitArrayTypeTraitExpr(ArrayTypeTraitExpr *node)
    {
        return true;
    }
     */

    /* Visit ExpressionTraitExpr
    bool VisitExpressionTraitExpr(ExpressionTraitExpr *node)
    {
        return true;
    }
     */

    /* Visit DependentScopeDeclRefExpr
    bool VisitDependentScopeDeclRefExpr(DependentScopeDeclRefExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXConstructExpr
    bool VisitCXXConstructExpr(CXXConstructExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXInheritedCtorInitExpr
    bool VisitCXXInheritedCtorInitExpr(CXXInheritedCtorInitExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXBindTemporaryExpr
    bool VisitCXXBindTemporaryExpr(CXXBindTemporaryExpr *node)
    {
        return true;
    }
     */

    /* Visit ExprWithCleanups
    bool VisitExprWithCleanups(ExprWithCleanups *node)
    {
        return true;
    }
     */

    /* Visit CXXTemporaryObjectExpr
    bool VisitCXXTemporaryObjectExpr(CXXTemporaryObjectExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXUnresolvedConstructExpr
    bool VisitCXXUnresolvedConstructExpr(CXXUnresolvedConstructExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXDependentScopeMemberExpr
    bool VisitCXXDependentScopeMemberExpr(CXXDependentScopeMemberExpr *node)
    {
        return true;
    }
     */

    /* Visit OverloadExpr
    bool VisitOverloadExpr(OverloadExpr *node)
    {
        return true;
    }
     */

    /* Visit UnresolvedLookupExpr
    bool VisitUnresolvedLookupExpr(UnresolvedLookupExpr *node)
    {
        return true;
    }
     */

    /* Visit UnresolvedMemberExpr
    bool VisitUnresolvedMemberExpr(UnresolvedMemberExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXNoexceptExpr
    bool VisitCXXNoexceptExpr(CXXNoexceptExpr *node)
    {
        return true;
    }
     */

    /* Visit PackExpansionExpr
    bool VisitPackExpansionExpr(PackExpansionExpr *node)
    {
        return true;
    }
     */

    /* Visit SizeOfPackExpr
    bool VisitSizeOfPackExpr(SizeOfPackExpr *node)
    {
        return true;
    }
     */

    /* Visit SubstNonTypeTemplateParmExpr
    bool VisitSubstNonTypeTemplateParmExpr(SubstNonTypeTemplateParmExpr *node)
    {
        return true;
    }
     */

    /* Visit SubstNonTypeTemplateParmPackExpr
    bool VisitSubstNonTypeTemplateParmPackExpr(SubstNonTypeTemplateParmPackExpr *node)
    {
        return true;
    }
     */

    /* Visit FunctionParmPackExpr
    bool VisitFunctionParmPackExpr(FunctionParmPackExpr *node)
    {
        return true;
    }
     */

    /* Visit MaterializeTemporaryExpr
    bool VisitMaterializeTemporaryExpr(MaterializeTemporaryExpr *node)
    {
        return true;
    }
     */

    /* Visit LambdaExpr
    bool VisitLambdaExpr(LambdaExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXFoldExpr
    bool VisitCXXFoldExpr(CXXFoldExpr *node)
    {
        return true;
    }
     */

    /* Visit CoroutineSuspendExpr
    bool VisitCoroutineSuspendExpr(CoroutineSuspendExpr *node)
    {
        return true;
    }
     */

    /* Visit CoawaitExpr
    bool VisitCoawaitExpr(CoawaitExpr *node)
    {
        return true;
    }
     */

    /* Visit DependentCoawaitExpr
    bool VisitDependentCoawaitExpr(DependentCoawaitExpr *node)
    {
        return true;
    }
     */

    /* Visit CoyieldExpr
    bool VisitCoyieldExpr(CoyieldExpr *node)
    {
        return true;
    }
     */

    /* Visit ConceptSpecializationExpr
    bool VisitConceptSpecializationExpr(ConceptSpecializationExpr *node)
    {
        return true;
    }
     */

    /* Visit RequiresExpr
    bool VisitRequiresExpr(RequiresExpr *node)
    {
        return true;
    }
     */

    /* Visit ObjCStringLiteral
    bool VisitObjCStringLiteral(ObjCStringLiteral *node)
    {
        return true;
    }
     */

    /* Visit ObjCBoxedExpr
    bool VisitObjCBoxedExpr(ObjCBoxedExpr *node)
    {
        return true;
    }
     */

    /* Visit ObjCArrayLiteral
    bool VisitObjCArrayLiteral(ObjCArrayLiteral *node)
    {
        return true;
    }
     */

    /* Visit ObjCDictionaryLiteral
    bool VisitObjCDictionaryLiteral(ObjCDictionaryLiteral *node)
    {
        return true;
    }
     */

    /* Visit ObjCEncodeExpr
    bool VisitObjCEncodeExpr(ObjCEncodeExpr *node)
    {
        return true;
    }
     */

    /* Visit ObjCMessageExpr
    bool VisitObjCMessageExpr(ObjCMessageExpr *node)
    {
        return true;
    }
     */

    /* Visit ObjCSelectorExpr
    bool VisitObjCSelectorExpr(ObjCSelectorExpr *node)
    {
        return true;
    }
     */

    /* Visit ObjCProtocolExpr
    bool VisitObjCProtocolExpr(ObjCProtocolExpr *node)
    {
        return true;
    }
     */

    /* Visit ObjCIvarRefExpr
    bool VisitObjCIvarRefExpr(ObjCIvarRefExpr *node)
    {
        return true;
    }
     */

    /* Visit ObjCPropertyRefExpr
    bool VisitObjCPropertyRefExpr(ObjCPropertyRefExpr *node)
    {
        return true;
    }
     */

    /* Visit ObjCIsaExpr
    bool VisitObjCIsaExpr(ObjCIsaExpr *node)
    {
        return true;
    }
     */

    /* Visit ObjCIndirectCopyRestoreExpr
    bool VisitObjCIndirectCopyRestoreExpr(ObjCIndirectCopyRestoreExpr *node)
    {
        return true;
    }
     */

    /* Visit ObjCBoolLiteralExpr
    bool VisitObjCBoolLiteralExpr(ObjCBoolLiteralExpr *node)
    {
        return true;
    }
     */

    /* Visit ObjCSubscriptRefExpr
    bool VisitObjCSubscriptRefExpr(ObjCSubscriptRefExpr *node)
    {
        return true;
    }
     */

    /* Visit ObjCAvailabilityCheckExpr
    bool VisitObjCAvailabilityCheckExpr(ObjCAvailabilityCheckExpr *node)
    {
        return true;
    }
     */

    /* Visit ObjCBridgedCastExpr
    bool VisitObjCBridgedCastExpr(ObjCBridgedCastExpr *node)
    {
        return true;
    }
     */

    /* Visit CUDAKernelCallExpr
    bool VisitCUDAKernelCallExpr(CUDAKernelCallExpr *node)
    {
        return true;
    }
     */

    /* Visit ShuffleVectorExpr
    bool VisitShuffleVectorExpr(ShuffleVectorExpr *node)
    {
        return true;
    }
     */

    /* Visit ConvertVectorExpr
    bool VisitConvertVectorExpr(ConvertVectorExpr *node)
    {
        return true;
    }
     */

    /* Visit BlockExpr
    bool VisitBlockExpr(BlockExpr *node)
    {
        return true;
    }
     */

    /* Visit OpaqueValueExpr
    bool VisitOpaqueValueExpr(OpaqueValueExpr *node)
    {
        return true;
    }
     */

    /* Visit TypoExpr
    bool VisitTypoExpr(TypoExpr *node)
    {
        return true;
    }
     */

    /* Visit RecoveryExpr
    bool VisitRecoveryExpr(RecoveryExpr *node)
    {
        return true;
    }
     */

    /* Visit BuiltinBitCastExpr
    bool VisitBuiltinBitCastExpr(BuiltinBitCastExpr *node)
    {
        return true;
    }
     */

    /* Visit MSPropertyRefExpr
    bool VisitMSPropertyRefExpr(MSPropertyRefExpr *node)
    {
        return true;
    }
     */

    /* Visit MSPropertySubscriptExpr
    bool VisitMSPropertySubscriptExpr(MSPropertySubscriptExpr *node)
    {
        return true;
    }
     */

    /* Visit CXXUuidofExpr
    bool VisitCXXUuidofExpr(CXXUuidofExpr *node)
    {
        return true;
    }
     */

    /* Visit SEHTryStmt
    bool VisitSEHTryStmt(SEHTryStmt *node)
    {
        return true;
    }
     */

    /* Visit SEHExceptStmt
    bool VisitSEHExceptStmt(SEHExceptStmt *node)
    {
        return true;
    }
     */

    /* Visit SEHFinallyStmt
    bool VisitSEHFinallyStmt(SEHFinallyStmt *node)
    {
        return true;
    }
     */

    /* Visit SEHLeaveStmt
    bool VisitSEHLeaveStmt(SEHLeaveStmt *node)
    {
        return true;
    }
     */

    /* Visit MSDependentExistsStmt
    bool VisitMSDependentExistsStmt(MSDependentExistsStmt *node)
    {
        return true;
    }
     */

    /* Visit AsTypeExpr
    bool VisitAsTypeExpr(AsTypeExpr *node)
    {
        return true;
    }
     */

    /* Visit DeclDecl
    bool VisitDeclDecl(DeclDecl *node)
    {
        return true;
    }
     */

    /* Visit TranslationUnitDecl
    bool VisitTranslationUnitDecl(TranslationUnitDecl *node)
    {
        return true;
    }
     */

    /* Visit PragmaCommentDecl
    bool VisitPragmaCommentDecl(PragmaCommentDecl *node)
    {
        return true;
    }
     */

    /* Visit PragmaDetectMismatchDecl
    bool VisitPragmaDetectMismatchDecl(PragmaDetectMismatchDecl *node)
    {
        return true;
    }
     */

    /* Visit ExternCContextDecl
    bool VisitExternCContextDecl(ExternCContextDecl *node)
    {
        return true;
    }
     */

    /* Visit NamedDecl
    bool VisitNamedDecl(NamedDecl *node)
    {
        return true;
    }
     */

    /* Visit NamespaceDecl
    bool VisitNamespaceDecl(NamespaceDecl *node)
    {
        return true;
    }
     */

    /* Visit UsingDirectiveDecl
    bool VisitUsingDirectiveDecl(UsingDirectiveDecl *node)
    {
        return true;
    }
     */

    /* Visit NamespaceAliasDecl
    bool VisitNamespaceAliasDecl(NamespaceAliasDecl *node)
    {
        return true;
    }
     */

    /* Visit LabelDecl
    bool VisitLabelDecl(LabelDecl *node)
    {
        return true;
    }
     */

    /* Visit TypeDecl
    bool VisitTypeDecl(TypeDecl *node)
    {
        return true;
    }
     */

    /* Visit TypedefNameDecl
    bool VisitTypedefNameDecl(TypedefNameDecl *node)
    {
        return true;
    }
     */

    /* Visit TypedefDecl
    bool VisitTypedefDecl(TypedefDecl *node)
    {
        return true;
    }
     */

    /* Visit TypeAliasDecl
    bool VisitTypeAliasDecl(TypeAliasDecl *node)
    {
        return true;
    }
     */

    /* Visit ObjCTypeParamDecl
    bool VisitObjCTypeParamDecl(ObjCTypeParamDecl *node)
    {
        return true;
    }
     */

    /* Visit UnresolvedUsingTypenameDecl
    bool VisitUnresolvedUsingTypenameDecl(UnresolvedUsingTypenameDecl *node)
    {
        return true;
    }
     */

    /* Visit TagDecl
    bool VisitTagDecl(TagDecl *node)
    {
        return true;
    }
     */

    bool VisitEnumDecl(EnumDecl *node)
    {
        string nodename = node->getNameAsString();
        if (nodename.length() > 0) {
            for (clang::CapturedDecl::specific_decl_iterator<EnumConstantDecl> decllt = node->enumerator_begin(); decllt != node->enumerator_end(); decllt++) {
                EnumConstantDecl *tmp = *decllt;
                string tmpname = tmp->getNameAsString();
                if (!isPrefixOf(tmpname, nodename)) {
                    SourceLocation loc = tmp->getLocation();
                    string message = "枚举值\'" + tmpname + "\'与声明的类型不对应，应该以\'" + nodename + "\'为前缀";
                    addViolation(loc, loc, this, message);
                }
            }
        }
        
        Decl *decl = node->getPreviousDecl();
        bool hasattr = node->hasAttrs();
        if (hasattr || decl) {
            return true;
        }
        SourceLocation loc = node->getLocation();
        string message = "请使用\'typedef NS_ENUM\'或者\'typedef NS_OPTIONS\'类型枚举";
        addViolation(loc, loc, this, message);
        return true;
    }
    
    ///获取当前上下文的编译源码
    string GrowingContextCode(TagDecl *node) {
        LangOptions LangOpts;
        LangOpts.ObjC = true;
        PrintingPolicy Policy(LangOpts);
        string sExpr;
        llvm::raw_string_ostream rsoExpr(sExpr);
        node->print(rsoExpr,Policy);
        return rsoExpr.str();
    }

    /* Visit RecordDecl
    bool VisitRecordDecl(RecordDecl *node)
    {
        return true;
    }
     */

    /* Visit CXXRecordDecl
    bool VisitCXXRecordDecl(CXXRecordDecl *node)
    {
        return true;
    }
     */

    /* Visit ClassTemplateSpecializationDecl
    bool VisitClassTemplateSpecializationDecl(ClassTemplateSpecializationDecl *node)
    {
        return true;
    }
     */

    /* Visit TemplateTypeParmDecl
    bool VisitTemplateTypeParmDecl(TemplateTypeParmDecl *node)
    {
        return true;
    }
     */

    /* Visit ValueDecl
    bool VisitValueDecl(ValueDecl *node)
    {
        return true;
    }
     */

    /* Visit EnumConstantDecl
    bool VisitEnumConstantDecl(EnumConstantDecl *node)
    {
        return true;
    }
     */

    /* Visit UnresolvedUsingValueDecl
    bool VisitUnresolvedUsingValueDecl(UnresolvedUsingValueDecl *node)
    {
        return true;
    }
     */

    /* Visit IndirectFieldDecl
    bool VisitIndirectFieldDecl(IndirectFieldDecl *node)
    {
        return true;
    }
     */

    /* Visit BindingDecl
    bool VisitBindingDecl(BindingDecl *node)
    {
        return true;
    }
     */

    /* Visit OMPDeclareReductionDecl
    bool VisitOMPDeclareReductionDecl(OMPDeclareReductionDecl *node)
    {
        return true;
    }
     */

    /* Visit OMPDeclareMapperDecl
    bool VisitOMPDeclareMapperDecl(OMPDeclareMapperDecl *node)
    {
        return true;
    }
     */

    /* Visit MSGuidDecl
    bool VisitMSGuidDecl(MSGuidDecl *node)
    {
        return true;
    }
     */

    /* Visit DeclaratorDecl
    bool VisitDeclaratorDecl(DeclaratorDecl *node)
    {
        return true;
    }
     */

    /* Visit FieldDecl
    bool VisitFieldDecl(FieldDecl *node)
    {
        return true;
    }
     */

    /* Visit ObjCIvarDecl
    bool VisitObjCIvarDecl(ObjCIvarDecl *node)
    {
        return true;
    }
     */

    /* Visit ObjCAtDefsFieldDecl
    bool VisitObjCAtDefsFieldDecl(ObjCAtDefsFieldDecl *node)
    {
        return true;
    }
     */

    /* Visit MSPropertyDecl
    bool VisitMSPropertyDecl(MSPropertyDecl *node)
    {
        return true;
    }
     */

    /* Visit FunctionDecl
    bool VisitFunctionDecl(FunctionDecl *node)
    {
        return true;
    }
     */

    /* Visit CXXDeductionGuideDecl
    bool VisitCXXDeductionGuideDecl(CXXDeductionGuideDecl *node)
    {
        return true;
    }
     */

    /* Visit CXXMethodDecl
    bool VisitCXXMethodDecl(CXXMethodDecl *node)
    {
        return true;
    }
     */

    /* Visit CXXConstructorDecl
    bool VisitCXXConstructorDecl(CXXConstructorDecl *node)
    {
        return true;
    }
     */

    /* Visit CXXDestructorDecl
    bool VisitCXXDestructorDecl(CXXDestructorDecl *node)
    {
        return true;
    }
     */

    /* Visit CXXConversionDecl
    bool VisitCXXConversionDecl(CXXConversionDecl *node)
    {
        return true;
    }
     */

    /// 检测到变量声明
    /// 1. static以及Extern修饰的NSString类型，以kGrowing开头
    bool VisitVarDecl(VarDecl *node)
    {
        string varname = node->getNameAsString();
        //通过 `getType` 获取类型，类型为 `NSString *const`
        QualType type = node->getType();
        //通过 `getInitStyle`  获取初始化类型，类型为 `cinit`
        clang::VarDecl::InitializationStyle initType = node->getInitStyle();
        if (initType == clang::VarDecl::CInit) {
            //判断是否 `Static`
            StorageClass stclass = node->getStorageClass();
            bool isStatic = (stclass == SC_Static || stclass == SC_Extern);
            string typeName = type.getAsString();
            //是否 `NSString *` 类型
            if (isPrefixOf("NSString *", typeName) && isStatic) {
            //获取配置的前缀名，默认为 `kGrowing`，并判断
                string prefix = RuleConfiguration::stringForKey("GIO_STATIC_NSSTRING_PREFIX","kGrowing");
                if (!isPrefixOf("kGrowing", varname)) {
                //获取 `location` 添加 `addViolation`
                    SourceLocation loc = node->getLocation();
                    string message = "static NSString*的变量: \'" + varname + "\' 前缀应使用" + prefix;
                    addViolation(loc, loc, this, message);
                }
            }
        }
        
        return true;
    }

    /* Visit VarTemplateSpecializationDecl
    bool VisitVarTemplateSpecializationDecl(VarTemplateSpecializationDecl *node)
    {
        return true;
    }
     */

    /* Visit ImplicitParamDecl
    bool VisitImplicitParamDecl(ImplicitParamDecl *node)
    {
        return true;
    }
     */

    /* Visit ParmVarDecl
    bool VisitParmVarDecl(ParmVarDecl *node)
    {
        return true;
    }
     */

    /* Visit DecompositionDecl
    bool VisitDecompositionDecl(DecompositionDecl *node)
    {
        return true;
    }
     */

    /* Visit OMPCapturedExprDecl
    bool VisitOMPCapturedExprDecl(OMPCapturedExprDecl *node)
    {
        return true;
    }
     */

    /* Visit NonTypeTemplateParmDecl
    bool VisitNonTypeTemplateParmDecl(NonTypeTemplateParmDecl *node)
    {
        return true;
    }
     */

    /* Visit TemplateDecl
    bool VisitTemplateDecl(TemplateDecl *node)
    {
        return true;
    }
     */

    /* Visit RedeclarableTemplateDecl
    bool VisitRedeclarableTemplateDecl(RedeclarableTemplateDecl *node)
    {
        return true;
    }
     */

    /* Visit FunctionTemplateDecl
    bool VisitFunctionTemplateDecl(FunctionTemplateDecl *node)
    {
        return true;
    }
     */

    /* Visit ClassTemplateDecl
    bool VisitClassTemplateDecl(ClassTemplateDecl *node)
    {
        return true;
    }
     */

    /* Visit VarTemplateDecl
    bool VisitVarTemplateDecl(VarTemplateDecl *node)
    {
        return true;
    }
     */

    /* Visit TypeAliasTemplateDecl
    bool VisitTypeAliasTemplateDecl(TypeAliasTemplateDecl *node)
    {
        return true;
    }
     */

    /* Visit TemplateTemplateParmDecl
    bool VisitTemplateTemplateParmDecl(TemplateTemplateParmDecl *node)
    {
        return true;
    }
     */

    /* Visit BuiltinTemplateDecl
    bool VisitBuiltinTemplateDecl(BuiltinTemplateDecl *node)
    {
        return true;
    }
     */

    /* Visit ConceptDecl
    bool VisitConceptDecl(ConceptDecl *node)
    {
        return true;
    }
     */

    /* Visit UsingDecl
    bool VisitUsingDecl(UsingDecl *node)
    {
        return true;
    }
     */

    /* Visit UsingPackDecl
    bool VisitUsingPackDecl(UsingPackDecl *node)
    {
        return true;
    }
     */

    /* Visit UsingShadowDecl
    bool VisitUsingShadowDecl(UsingShadowDecl *node)
    {
        return true;
    }
     */

    /* Visit ConstructorUsingShadowDecl
    bool VisitConstructorUsingShadowDecl(ConstructorUsingShadowDecl *node)
    {
        return true;
    }
     */

    bool VisitObjCMethodDecl(ObjCMethodDecl *node)
    {
        //检查名称的每部分，都不允许以大写字母开头
        Selector sel = node -> getSelector();
        string selectorName = sel.getAsString();
        bool isclassmtd = node->isClassMethod();
        if (isclassmtd) {
            QualType rt = node->getReturnType();
            if (rt.getAsString() == "id") {
                SourceLocation loc = node->getLocation();
                string message = "类方法: \'" + selectorName + "\' 应返回\'instancetype\'类型，不要使用\'id\'";
                addViolation(loc, loc, this, message);
            }
        }
        int selectorPartCount = node -> getNumSelectorLocs();
        
        for (int i = 0; i < selectorPartCount; i++) {
            // 方法参数名称
            StringRef selName = sel.getNameForSlot(i);
            if (selName.size() == 0) {
                return true;
            }
            string selstr = sel.getAsString();
            string allow_uppercase = RuleConfiguration::stringForKey("GIO_METHOD_ALLOW_UPPERCASE","URL");
            vector<string> v;
            SplitString(allow_uppercase,v,",");
            bool isallow = false;
            for (vector<string>::size_type i = 0; i != v.size(); ++i) {
                if (isPrefixOf(v[i], selstr) ) {
                    isallow = true;
                    break;
                }
            }
            
            char c = selName[0];
            if (isUppercase(c) && !isallow) {
                // 提示
                // 获取将要报错的位置
                SourceLocation loc = node->getSelectorLoc(i);
                string message = "方法名/方法参数: \'" + selectorName + "\' 不能以大写开头";
                addViolation(loc, loc, this, message);
            }
            //            if (c == '_') {
            //                // 提示
            //                // 获取将要报错的位置
            //                SourceLocation loc = node->getSelectorLoc(i);
            //                string message = "方法名/方法参数: \'" + selectorName + "\' 不要以下划线开头";
            //                addViolation(loc, loc, this, message);
            //            }
        }
        return true;
    }
    
    void SplitString(const string& s, vector<string>& v, const string& c)
    {
        string::size_type pos1, pos2;
        pos2 = s.find(c);
        pos1 = 0;
        while(string::npos != pos2)
        {
            v.push_back(s.substr(pos1, pos2-pos1));
            
            pos1 = pos2 + c.size();
            pos2 = s.find(c, pos1);
        }
        if(pos1 != s.length())
            v.push_back(s.substr(pos1));
    }

    /* Visit ObjCContainerDecl
    bool VisitObjCContainerDecl(ObjCContainerDecl *node)
    {
        return true;
    }
     */

    /* Visit ObjCCategoryDecl
    bool VisitObjCCategoryDecl(ObjCCategoryDecl *node)
    {
        return true;
    }
     */

    /* Visit ObjCProtocolDecl
    bool VisitObjCProtocolDecl(ObjCProtocolDecl *node)
    {
        return true;
    }
     */

    bool VisitObjCInterfaceDecl(ObjCInterfaceDecl *node)
    {
        ObjCInterfaceDecl* interface = node->getSuperClass();
        if (interface != NULL) {
            string currentname = node->getNameAsString();
            string supername = interface->getNameAsString();
            string prefix = RuleConfiguration::stringForKey("GIO_CLASS_PREFIX","Growing");
            if (isPrefixOf("NS", supername) || isPrefixOf("UI", supername)) {
                if (!isPrefixOf(prefix, currentname)) {
                    SourceLocation loc = node->getLocation();
                    string message = "自定义类命名: \'" + currentname + "\' 需要以" + prefix + "开头";
                    addViolation(loc, loc, this, message);
                }
            }
        }
        return true;
    }

    ///检测到分类
    bool VisitObjCCategoryImplDecl(ObjCCategoryImplDecl *node)
    {
        StringRef namearr = node->getName();
        string categoryname = node->getNameAsString();
        for (int i = 0; i < categoryname.length(); i++) {
            char c = namearr[i];
            if (c == '_') {
                // 提示
                // 获取将要报错的位置
                SourceLocation loc = node->getCategoryNameLoc();
                string message = "分类命名: \'" + categoryname + "\' 不要包含下划线";
                addViolation(loc, loc, this, message);
                break;
            }
        }
        ObjCInterfaceDecl* interface = node->getClassInterface();
        if (interface != NULL) {
            string interfacename = interface->getNameAsString();
            string prefix = RuleConfiguration::stringForKey("GIO_CATEGORY_PREFIX","Growing");
            if (isPrefixOf("NS", interfacename) || isPrefixOf("UI", interfacename)) {
                if (!isPrefixOf(prefix, categoryname)) {
                    SourceLocation loc = node->getCategoryNameLoc();
                    string message = "系统分类命名: \'" + categoryname + "\' 需要以" + prefix + "开头";
                    addViolation(loc, loc, this, message);
                }
            }
        }
        
        return true;
    }
    
    bool isPrefixOf(string a,string b){
        return a.size()<b.size()&&(b.compare(0,a.size(),a)==0);
    }

    /* Visit ObjCImplementationDecl
    bool VisitObjCImplementationDecl(ObjCImplementationDecl *node)
    {
        return true;
    }
     */

    ///检测到属性，不管是谁的属性
    bool VisitObjCPropertyDecl(ObjCPropertyDecl *node)
    {
        string propertyname = node->getNameAsString();
        QualType type = node->getType();
        string ivarname = type.getAsString();
        if (isPrefixOf("NSString", ivarname)) {
            int kind = node->getPropertyAttributes();
            if (kind & 0x20) {
                //nothing
            } else {
                SourceLocation loc = node->getLocation();
                string message = "\'NSString\'类型的属性: \'" + propertyname + "\' 应该以copy修饰";
                addViolation(loc, loc, this, message);
            }
        }
        string str[] = {"NSArray", "NSDictionary", "NSSet"};
        vector<string> v(str,str+sizeof(str)/sizeof(str[0]));
        
        
        string templateStr[] = {"NSArray<NSNumber *>*", "NSDictionary<NSString *, NSDate *>*", "NSSet<NSNumber *>*"};
        vector<string> templateV(templateStr,templateStr+sizeof(templateStr)/sizeof(templateStr[0]));
        
        for (vector<string>::size_type i = 0; i != v.size(); ++i) {
            string classname = v[i];
            if (isPrefixOf(classname, ivarname)) {
                if (ivarname.find("<") == ivarname.npos && ivarname.find(">") == ivarname.npos) {
                    SourceLocation loc = node->getLocation();
                    string message = classname + " 集合的属性: \'" + propertyname + "\' 应该以指定泛型. eg:" + templateV[i];
                    addViolation(loc, loc, this, message);
                }
            }
        }
        
        
        StringRef nameref = node->getName();
        if (isUppercase(nameref[0])) {
            SourceLocation loc = node->getLocation();
            string message = "属性: \'" + propertyname + "\' 不能以大写开头";
            addViolation(loc, loc, this, message);
        }
        
        return true;
    }


    /* Visit ObjCCompatibleAliasDecl
    bool VisitObjCCompatibleAliasDecl(ObjCCompatibleAliasDecl *node)
    {
        return true;
    }
     */

    /* Visit LinkageSpecDecl
    bool VisitLinkageSpecDecl(LinkageSpecDecl *node)
    {
        return true;
    }
     */

    /* Visit ExportDecl
    bool VisitExportDecl(ExportDecl *node)
    {
        return true;
    }
     */

    /* Visit ObjCPropertyImplDecl
    bool VisitObjCPropertyImplDecl(ObjCPropertyImplDecl *node)
    {
        return true;
    }
     */

    /* Visit FileScopeAsmDecl
    bool VisitFileScopeAsmDecl(FileScopeAsmDecl *node)
    {
        return true;
    }
     */

    /* Visit AccessSpecDecl
    bool VisitAccessSpecDecl(AccessSpecDecl *node)
    {
        return true;
    }
     */

    /* Visit FriendDecl
    bool VisitFriendDecl(FriendDecl *node)
    {
        return true;
    }
     */

    /* Visit FriendTemplateDecl
    bool VisitFriendTemplateDecl(FriendTemplateDecl *node)
    {
        return true;
    }
     */

    /* Visit StaticAssertDecl
    bool VisitStaticAssertDecl(StaticAssertDecl *node)
    {
        return true;
    }
     */

    /* Visit BlockDecl
    bool VisitBlockDecl(BlockDecl *node)
    {
        return true;
    }
     */

    /* Visit CapturedDecl
    bool VisitCapturedDecl(CapturedDecl *node)
    {
        return true;
    }
     */

    /* Visit ClassScopeFunctionSpecializationDecl
    bool VisitClassScopeFunctionSpecializationDecl(ClassScopeFunctionSpecializationDecl *node)
    {
        return true;
    }
     */

    /* Visit ImportDecl
    bool VisitImportDecl(ImportDecl *node)
    {
        return true;
    }
     */

    /* Visit OMPThreadPrivateDecl
    bool VisitOMPThreadPrivateDecl(OMPThreadPrivateDecl *node)
    {
        return true;
    }
     */

    /* Visit OMPAllocateDecl
    bool VisitOMPAllocateDecl(OMPAllocateDecl *node)
    {
        return true;
    }
     */

    /* Visit OMPRequiresDecl
    bool VisitOMPRequiresDecl(OMPRequiresDecl *node)
    {
        return true;
    }
     */

    /* Visit EmptyDecl
    bool VisitEmptyDecl(EmptyDecl *node)
    {
        return true;
    }
     */

    /* Visit RequiresExprBodyDecl
    bool VisitRequiresExprBodyDecl(RequiresExprBodyDecl *node)
    {
        return true;
    }
     */

    /* Visit LifetimeExtendedTemporaryDecl
    bool VisitLifetimeExtendedTemporaryDecl(LifetimeExtendedTemporaryDecl *node)
    {
        return true;
    }
     */

};

static RuleSet rules(new GrowingIOCodeCheckerRule());
