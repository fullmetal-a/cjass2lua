#pragma once

#include "defs.h"
#include <memory>
#include <Windows.h>
#include <set>
#include <functional>

#include "OutputInterface.h"

namespace cJass
{
	class Node
	{
	public:
		enum class Type
		{
			Undefined,
			RootNode,
			Function,
			Comment,
			OperationObject,
			LocalDeclaration,
			GlobalDeclaration,
			TypeDef,
			Class,
			Method
		};

		friend class Comment;
		friend class Function;
		friend class OperationObject;
		friend class LocalDeclaration;
		friend class GlobalDeclaration;
		friend class RootNode;
		friend class Class;
		friend class Method;

	protected:
		bool				_isNewLine;
		size_t				_tabs;
		OutputInterface&	_out;
		NodeList			_subnodes;
		int					_depthIndex;
		bool				_isBlock;
		bool				_isComplete;
		size_t				_topIndex;
		Node*				_top;
		bool				_isString;
		bool				_isWrapper;
		RootNode*			_root;

	private:
		Type				_type;
		NL_iter				_it;

	protected:
		virtual void CountAllNodes() = 0;

		Node(Node&) = delete;

	public:
		static NodePtr Produce(Node::Type type, Node* top, OutputInterface& outIf);

		static std::string ToString(Node* node);

		Node(OutputInterface& outIf, Type type = Type::Undefined, Node* top = nullptr);
		virtual ~Node();

		void AddSubnode(NodePtr node);
		Type GetType() const;
		size_t CountSubnodes() const;
		Node* Top() const;
		Node* LastSubnode();
		void PrintTabs(int substract = 0);
		size_t GetDepth() const;
		bool IsBlock() const;
		bool IsComplete() const;
		void Complete(bool isComplete);
		bool IsString() const;
		bool IsWrapper() const;

		NodePtr IterateSubnodes();
		NodePtr AtNode(size_t i);
		void ResetIterator();
		template <class PtrType = Node> PtrType* Ptr() { return dynamic_cast<PtrType*>(this); }

		virtual void ToLua() = 0;
		virtual void InitData(const std::vector<std::string>& strings) = 0;
		virtual void InsertStringId(const std::string& strId) {}
		virtual bool HasStringId(const std::string& strId) const { return false;  }
		virtual std::string GetText() const { return ""; }
		Node* GetAdjacentNode(bool right) const;
	};

	class RootNode : public Node
	{
	public:
		friend class Node;
		friend class Function;
		friend class OperationObject;
		friend class LocalDeclaration;
		friend class GlobalDeclaration;
		friend class Class;
		friend class Method;

	private:
		std::set<std::string>								_globalIdSet;
		std::vector<std::string>							_strIds;
		int													_totalNodes;
		int													_printedNodes;
		std::shared_ptr<std::function<void(int,int)>>		_notifyCallback;
		bool												_useCallback;
	
	protected:
		void IncrementNodeCount(size_t i);
		virtual void CountAllNodes() override;
		void AddGlobalId(const std::string& id);
		void AssertGlobalId(const std::string& id);
		 
	public:
		RootNode(OutputInterface& outIf);
		virtual void ToLua() override;
		virtual void InitData(const std::vector<std::string>& strings) override;
		void InsertStringId(const std::string& strId) override;
		bool HasStringId(const std::string& str) const override;
		void TryToCreatePrintNotify();
		void AddNotifyCallback(NotifyCallback callback);
		void Clear();
	};

	class Comment : public Node
	{
	private:
		std::string _comment;

	protected:
		virtual void CountAllNodes() override;

	public:
		Comment(OutputInterface& outIf, Node* top);
		virtual void ToLua() override;
		virtual void InitData(const std::vector<std::string>& strings) override;
	};

	class GlobalDeclaration : public Node
	{
	private:
		std::string		_varType;
		std::string		_varName;
		bool			_isNative;
		bool			_isArray;
		
	protected:
		virtual void CountAllNodes() override;

	public:
		friend class Node;

		GlobalDeclaration(OutputInterface& outIf, Node* top);

		virtual void ToLua() override;
		virtual void InitData(const std::vector<std::string>& strings) override;
	};

	class TypeDef : public Node
	{
	private:
		std::string		_nativeName;
		std::string		_extends;

	protected:
		virtual void CountAllNodes() override;

	public:
		TypeDef(OutputInterface& outIf, Node* top);

		virtual void ToLua() override;
		virtual void InitData(const std::vector<std::string>& strings) override;
	};

	class Class : public Node
	{
	public:
		struct ClassMember_t
		{
			std::string type;
			std::string name;
			std::string initValue;
			bool		isArray;
		};

	private:
		std::string											_className;
		std::vector<ClassMember_t>							_members;
		std::vector<ClassMember_t>							_staticMembers;
		std::vector<std::string>							_memberList;
		std::vector<std::string>							_strIds;

	protected:
		virtual void CountAllNodes() override;

	public:
		friend class Method;

		Class(OutputInterface& outIf, Node* top);
		virtual void ToLua() override;
		virtual void InitData(const std::vector<std::string>& strings) override;
		void RegMember(const std::vector<std::string>& mem, bool isStatic);
		Node* RegMethod(const std::vector<std::string>& met, bool isStatic);
		std::string GetClassName() const;
		bool HasNonstaticMember(const std::string& name) const;
	};

	class Method : public Node
	{
	private:
		std::string					_name;
		std::string					_thisName;
		std::string					_returnType;
		std::vector<std::string>	_args;
		std::vector<std::string>	_strIds;
		bool						_isStatic;

	protected:
		virtual void CountAllNodes() override;
		bool isStatic() const;

	public:  
		friend class Class;

		Method(OutputInterface& outIf, Node* top);
		virtual void ToLua() override;
		virtual void InitData(const std::vector<std::string>& strings) override;
		void InsertStringId(const std::string& strId) override;
		bool HasStringId(const std::string& strId) const override;
		bool IsStatic() const;
		std::string GetMethodName() const;
		void SetThisname(const std::string& name);
		std::string GetReplaceThisName() const;
	};


	class Function : public Node
	{
	protected:
		std::string					_name;

	private:
		std::string					_returnType;
		std::vector<std::string>	_args;
		std::vector<std::string>	_strIds;
		bool						_isNative;

	protected:
		virtual void CountAllNodes() override;

	public:
		friend class Node;

		Function(OutputInterface& outIf, Node* top);
		virtual void ToLua() override;
		virtual void InitData(const std::vector<std::string>& strings) override;
		void InsertStringId(const std::string& strId) override;
		bool HasStringId(const std::string& strId) const override;
	};

	class OperationObject : public Node
	{
	public:
		enum class OpType
		{
			Unknown,
			
			//Fundamental
			Id,
			Constant,
			Operator,
			UnaryOperator,
			NewLine,
			empty,

			//Expressions
			ExitWhen,
			DoStatement,
			Return,
			Expression,			//General expression in () brackets or without them
			Call,				//Function call
			Index,				//Array index expression [] (not definition)
			
			//Blocks
			If,
			Elseif,
			Else,
			While,
			WhileNot,
			Do,
			Loop,
			Lambda,
			For,

			//Special wrappers
			Wrapper,			//General wrapper for operations like call, assignment and return
			Logic,				//Wrapper for logical blocks
			VarInitExpression,  //Wrapper for local variable initializer
			Argument,			//Wrapper for an argument of function call
			GVarInitWrapper		//Wrapper to init global variable
		};

		enum class ConstType
		{
			Undefined,
			Integer,
			RawCode,
			Float,
			Bool,
			String,
			null
		};

	private:
		std::string		_opText;
		std::string		_extra;
		OpType			_otype;
		bool			_inBrackets;
		bool			_unaryExpr;
		bool			_lambdaIsSingle;
		bool			_blockClosed;
		ConstType		_cType;
		bool			_AssignUnary;

	protected:
		virtual void CountAllNodes() override;
		bool HasNeighbourStrings();
		std::string GetFullIdName();	//For adding this. in class members/functions

	public:
		OperationObject(OutputInterface& outIf, Node* top);
		virtual void ToLua() override;
		virtual void InitData(const std::vector<std::string>& strings) override;
		OpType GetOpType() const;
		bool isEmpty() const;
		bool LambdaIsSingle() const;
		bool BlockClosed() const;
		void CloseBlock();
		bool CheckIsString();
		ConstType GetConstType() const;
		virtual std::string GetText() const override;
	};

	class LocalDeclaration : public Node
	{
	private:
		std::string					 _type;
		std::vector<std::string>	 _vars;
		std::vector<std::string>	_arrSizes;
		bool						_doNotPrint;

	protected:
		virtual void CountAllNodes() override;

	public:
		LocalDeclaration(OutputInterface& outIf, Node* top);
		virtual void ToLua() override;
		virtual void InitData(const std::vector<std::string>& strings) override;
		size_t GetVarCount() const;
		Node* AddVariable(const std::string& name, const std::string& arrSize = 0);
		bool hasType() const;
		void DoNotPrint();
	};
}