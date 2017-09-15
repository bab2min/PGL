#pragma once

#include "PGLFirstData.h"
#include "Error.h"

struct PGL_closeUnit;
struct PGL_loopUnit;
struct PGL_tree;
struct FinalData;

struct PGLID
{
	PGL_closeUnit* belong;
	int addr;
	int raddr;
	bool ref;
	bool constant;
	shared_ptr<PGLID> globFunc;

	PGLID() : globFunc(nullptr) {}
};

struct PGLIDList
{
	PGL_closeUnit* belong;
	map<string, shared_ptr<PGLID>> id;
	PGLIDList() : belong(nullptr) {}
};

struct ResolveData
{
	map<string, shared_ptr<PGLID>> glob;
	map<PGL_closeUnit*, PGLIDList> idList;
	map<shared_ptr<PGLCompoundData>, shared_ptr<PGLID>, PGLDataCompare> literalList;
	vector<PGL_closeUnit*> scope;
	ErrorCnt* err;
	size_t globListCount;
	ResolveData() : globListCount(0) {}
	PGLIDList& GetIDList() {return idList[scope.back()];}
	shared_ptr<PGLID> RegisterGlobal(string name, const shared_ptr<PGLCompoundData>& data);
	string GetScopePrefix() const;
};

struct GenerateData
{
	ResolveData* rd;
	vector<PGL_loopUnit*> loopScope;
	map<PGL_closeUnit*, vector<short>> codes;
	map<PGL_closeUnit*, vector<PGL_tree*>> debugInfo;
	vector<PGL_closeUnit*> cur;
	vector<short>& GetCurrentCode() {return codes[cur.back()];}
	vector<PGL_tree*>& GetCurrentDebugInfo() {return debugInfo[cur.back()];}
};

struct PGL_tree
{
	int line;
	virtual void ResolveIdentifier(ResolveData* rd) {}
	virtual void GenerateCode(GenerateData* gd) {}
	virtual string GetType() const {return "";}
};

struct PGL_closeUnit
{
	int stacked;
	bool memberFunc;
	map<string, shared_ptr<PGLID>> captureList;
	PGL_closeUnit() : memberFunc(false), stacked(0) {}
	virtual void Link(int pos) {}
	virtual string GetName() const {return "";}
};

struct PGL_loopUnit
{
	vector<size_t> statBreak;
	vector<size_t> statContinue;
	void Solve(GenerateData* gd, size_t continuePos, size_t elsePos);
};

struct PGL_expr : public PGL_tree
{
	virtual ~PGL_expr() {}
};

struct PGL_lexpr : public PGL_expr
{
	virtual void GenerateLCode(GenerateData* gd) {}
	virtual ~PGL_lexpr() {}
};

struct PGL_literal : public PGL_expr
{
	shared_ptr<PGLCompoundData> l;
	PGL_literal(shared_ptr<PGLCompoundData> _l) : l(_l) {}
	void ResolveIdentifier(ResolveData* rd);
	void GenerateCode(GenerateData* gd);
};

struct PGL_identifier : public PGL_lexpr
{
	string id;
	shared_ptr<PGLID> pid;
	PGL_identifier(string _id) : id(_id), pid(nullptr) {}
	void ResolveIdentifier(ResolveData* rd);
	void GenerateCode(GenerateData* gd);
	void GenerateLCode(GenerateData* gd);
};

struct PGL_this : public PGL_lexpr
{
	PGL_this() {}
	void ResolveIdentifier(ResolveData* rd);
	void GenerateCode(GenerateData* gd);
	void GenerateLCode(GenerateData* gd);
};

struct PGL_not_expr : public PGL_expr
{
	shared_ptr<PGL_expr> r;
	PGL_not_expr(shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_and_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_and_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_or_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_or_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_equal_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_equal_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_notequal_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_notequal_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_greater_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_greater_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_greaterequal_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_greaterequal_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_less_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_less_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_lessequal_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_lessequal_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_add_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_add_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_sub_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_sub_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_mul_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_mul_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_div_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_div_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_mod_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_mod_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_pow_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l, r;
	PGL_pow_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_sign_expr : public PGL_expr
{
	shared_ptr<PGL_expr> r;
	PGL_sign_expr(shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_dot_expr : public PGL_lexpr
{
	shared_ptr<PGL_expr> l;
	string r;
	shared_ptr<PGLID> pid;
	PGL_dot_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		string _r = string()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd);
	void GenerateCode(GenerateData* gd);
	void GenerateLCode(GenerateData* gd);
	virtual string GetType() const {return "dot_expr";}
};

struct PGL_ref_expr : public PGL_lexpr
{
	shared_ptr<PGL_expr> l, r;
	PGL_ref_expr(shared_ptr<PGL_expr> _l = shared_ptr<PGL_expr>(),
		shared_ptr<PGL_expr> _r = shared_ptr<PGL_expr>()) : l(_l), r(_r) {}
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
	void GenerateLCode(GenerateData* gd);
};

struct PGL_functioncall_expr : public PGL_expr
{
	shared_ptr<PGL_expr> l;
	vector<shared_ptr<PGL_expr>> params;
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); for(auto&r : params)r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_array_expr : public PGL_expr
{
	vector<shared_ptr<PGL_expr>> elem;
	void ResolveIdentifier(ResolveData* rd) {for(auto&r : elem)r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_key_val
{
	shared_ptr<PGL_expr> key;
	shared_ptr<PGL_expr> val;
	void ResolveIdentifier(ResolveData* rd) {key->ResolveIdentifier(rd); val->ResolveIdentifier(rd);}
};

struct PGL_dict_expr : public PGL_expr
{
	vector<PGL_key_val> elem;
	void ResolveIdentifier(ResolveData* rd) {for(auto&r : elem)r.ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_sentence : public PGL_tree
{
	virtual ~PGL_sentence() {}
	virtual void CollectID(ResolveData* rd) {}
	virtual void EraseIdentifier(ResolveData* rd) {}
	virtual int CountPopStack() const {return 0;}
};

struct PGL_simple_sentence : public PGL_sentence
{

};

struct PGL_assign : public PGL_simple_sentence
{
	shared_ptr<PGL_lexpr> l;
	shared_ptr<PGL_expr> r;
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd); r->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_call : public PGL_simple_sentence
{
	shared_ptr<PGL_expr> l;
	void ResolveIdentifier(ResolveData* rd) {l->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_import
{
	string name;
};

struct PGL_whole
{
	vector<shared_ptr<PGL_sentence>> sents;
	vector<PGL_import> imports;
	void CollectID(ResolveData* rd) {for(auto& r : sents)r->CollectID(rd);}
	void ResolveIdentifier(ResolveData* rd) {for(auto& r : sents)r->ResolveIdentifier(rd);for(auto& r : sents)r->EraseIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_block : public PGL_whole
{
	void GenerateCode(GenerateData* gd);
	int CountPopStack() const {return accumulate(sents.begin(), sents.end(), 0, [](int a, const shared_ptr<PGL_sentence>& b){return a + b->CountPopStack();});}
};

struct PGL_if : public PGL_sentence
{
	shared_ptr<PGL_expr> cond;
	PGL_block trueSents, falseSents;
	void ResolveIdentifier(ResolveData* rd) {cond->ResolveIdentifier(rd); trueSents.ResolveIdentifier(rd); falseSents.ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_while : public PGL_sentence, public PGL_loopUnit
{
	shared_ptr<PGL_expr> cond;
	PGL_block sents;
	PGL_block doneS;
	PGL_block elseS;
	void ResolveIdentifier(ResolveData* rd) {cond->ResolveIdentifier(rd); sents.ResolveIdentifier(rd); doneS.ResolveIdentifier(rd); elseS.ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_return : public PGL_sentence
{
	shared_ptr<PGL_expr> ret;
	void ResolveIdentifier(ResolveData* rd) {ret->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_yield : public PGL_sentence
{
	shared_ptr<PGL_expr> ret;
	void ResolveIdentifier(ResolveData* rd) {ret->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_throw : public PGL_sentence
{
	shared_ptr<PGL_expr> ret;
	void ResolveIdentifier(ResolveData* rd) {ret->ResolveIdentifier(rd);}
	void GenerateCode(GenerateData* gd);
};

struct PGL_break : public PGL_sentence
{
	string label;
	void GenerateCode(GenerateData* gd);
};

struct PGL_continue : public PGL_sentence
{
	string label;
	void GenerateCode(GenerateData* gd);
};

struct PGL_decl_var : public PGL_sentence
{
	string name, type;
	shared_ptr<PGL_expr> init;
	shared_ptr<PGLID> pid;
	void CollectID(ResolveData* rd);
	void ResolveIdentifier(ResolveData* rd);
	void EraseIdentifier(ResolveData* rd);
	void GenerateCode(GenerateData* gd);
	int CountPopStack() const {return 1;}
};


struct PGL_try : public PGL_sentence
{
	string catchName;
	PGL_block trySents, catchSents;
	shared_ptr<PGLID> pid;
	void ResolveIdentifier(ResolveData* rd);
	void GenerateCode(GenerateData* gd);
};

struct PGL_for : public PGL_sentence, public PGL_loopUnit//, public PGL_closeUnit
{
	shared_ptr<PGL_decl_var> var;
	shared_ptr<PGL_expr> cond;
	shared_ptr<PGL_simple_sentence> loop;
	PGL_block sents;
	PGL_block doneS;
	PGL_block elseS;
	void ResolveIdentifier(ResolveData* rd);
	void GenerateCode(GenerateData* gd);
};

struct PGL_for_in : public PGL_sentence, public PGL_loopUnit//, public PGL_closeUnit
{
	string varKey, varVal;
	shared_ptr<PGLID> pidCont;
	shared_ptr<PGLID>pidIt;
	shared_ptr<PGLID> pidKey;
	shared_ptr<PGLID> pidVal;
	shared_ptr<PGL_expr> container;
	PGL_block sents;
	PGL_block doneS;
	PGL_block elseS;
	void ResolveIdentifier(ResolveData* rd);
	void GenerateCode(GenerateData* gd);
};

struct PGL_param
{
	shared_ptr<PGLID> pid;
	string name, type;
};

struct PGL_decl_function : public PGL_sentence, public PGL_closeUnit
{
	string name, type;
	vector<PGL_param> params;
	PGL_block sents;
	shared_ptr<PGLID> pid;
	shared_ptr<PGLFunctionData> faddr;
	void CollectID(ResolveData* rd);
	void ResolveIdentifier(ResolveData* rd);
	void EraseIdentifier(ResolveData* rd);
	void GenerateCode(GenerateData* gd);
	void Link(int pos);
	string GetName() const {return name;}
};

struct PGL_function_expr : public PGL_expr, public PGL_closeUnit
{
	string type;
	vector<PGL_param> params;
	PGL_block sents;
	shared_ptr<PGLID> pid;
	shared_ptr<PGLFunctionData> faddr;
	void ResolveIdentifier(ResolveData* rd);
	void GenerateCode(GenerateData* gd);
	void Link(int pos);
};

FinalData PGLLink(GenerateData* gd);