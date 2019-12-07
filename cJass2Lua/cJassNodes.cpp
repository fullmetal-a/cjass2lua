#include "cJassNodes.h"
#include "reutils.h"
#include "Settings.h"
#include "Utils.h"
#include <cmath>

#define d if (1 == 2) \

#define PRODUCING_NODE(__node__) case Node::Type::##__node__: \
			return std::shared_ptr<__node__>(new __node__(outIf, top))

int globalTabMinus = 0;

namespace cJass
{
	static OperationObject::ConstType g_prevConst = OperationObject::ConstType::Undefined;

	NodePtr Node::Produce(Node::Type type, Node* top, OutputInterface& outIf)
	{
		switch (type)
		{
			PRODUCING_NODE(Function);
			PRODUCING_NODE(Comment);
			PRODUCING_NODE(OperationObject);
			PRODUCING_NODE(LocalDeclaration);
			PRODUCING_NODE(GlobalDeclaration);
			PRODUCING_NODE(TypeDef);
			PRODUCING_NODE(Class);
			PRODUCING_NODE(Method);
		default:
			throw std::runtime_error("Error! Node RootNode cannot be produced!");
		}
	}

	std::string Node::ToString(Node* node)
	{
		std::string res;
		switch (node->_type)
		{
		case Type::Comment:
			return "Comment";
		
		case Type::Function:
			return "Function";

		case Type::RootNode:
			return "RootNode";

		case Type::LocalDeclaration:
			return "LocalDeclaration";

		case Type::OperationObject:
			res = "OperationObject(";
			switch (node->Ptr<OperationObject>()->GetOpType())
			{
			case OperationObject::OpType::Argument:
				res += "Argument";
				break;

			case OperationObject::OpType::Call:
				res += "Call";
				break;

			case OperationObject::OpType::Constant:
				res += "Constant";
				break;

			case OperationObject::OpType::Else:
				res += "Else";
				break;

			case OperationObject::OpType::Elseif:
				res += "Elseif";
				break;

			case OperationObject::OpType::Expression:
				res += "Expression";
				break;

			case OperationObject::OpType::Id:
				res += "Id";
				break;

			case OperationObject::OpType::If:
				res += "If";
				break;

			case OperationObject::OpType::Index:
				res += "Index";
				break;

			case OperationObject::OpType::Lambda:
				res += "Lambda";
				break;

			case OperationObject::OpType::Operator:
				res += "Operator";
				break;

			case OperationObject::OpType::Return:
				res += "Return";
				break;

			case OperationObject::OpType::UnaryOperator:
				res += "UnaryOperator";
				break;

			case OperationObject::OpType::Unknown:
				res += "Unknown";
				break;

			case OperationObject::OpType::VarInitExpression:
				res += "VarInitExpression";
				break;

			case OperationObject::OpType::While:
				res += "While";
				break;

			case OperationObject::OpType::Wrapper:
				res += "Wrapper";
				break;

			case OperationObject::OpType::Logic:
				res += "Logic";
				break;

			case OperationObject::OpType::WhileNot:
				res += "WhileNot";
				break;

			case OperationObject::OpType::Do:
				res += "Do";
				break;

			case OperationObject::OpType::DoStatement:
				res += "DoStatement";
				break;

			case OperationObject::OpType::ExitWhen:
				res += "ExitWhen";
				break;

			default:
				res += "Unknown";
			}
			res += ")";
			return res;

		default:
			return "Undefined";
		}
	}

	Node::Node(OutputInterface& outIf, Type type, Node* top)
		: _type(type)
		, _out(outIf)
		, _top(top)
		, _isNewLine(false)
		, _tabs(0)
		, _depthIndex(0)
		, _isBlock(false)
		, _isComplete(true)
		, _topIndex(0)
		, _isString(false)
		, _isWrapper(false)
		, _root(nullptr)
	{
		if (top != nullptr)
		{
			_depthIndex = top->_depthIndex;
			_root = top->_root;
			if (top->_subnodes.size() > 0)
				_topIndex = top->_subnodes.size() - 1;
		}
	}

	Node::~Node()
	{
	}

	void Node::AddSubnode(NodePtr node)
	{
		if (node->_depthIndex == 0)
			node->_depthIndex = _depthIndex + 1;

		if (this->GetType() == Type::RootNode)
		{
			if (node->GetType() == Type::Function)
				_root->AddGlobalId(node->Ptr<Function>()->_name);
			else if (node->GetType() == Type::GlobalDeclaration)
				_root->AddGlobalId(node->Ptr<GlobalDeclaration>()->_varName);
		}
		
		node->_root = _root;
		_subnodes.push_back(node);
		ResetIterator();
	}

	Node::Type Node::GetType() const
	{
		return _type;
	}

	NodePtr Node::IterateSubnodes()
	{
		if (_it == _subnodes.begin())
			return *(_it++);
		else if (_it == _subnodes.end())
			return nullptr;
		else
			return *(++_it);
	}

	void Node::ResetIterator()
	{
		_it = _subnodes.begin();
	}

	Node* Node::Top() const
	{
		return _top;
	}

	Node* Node::GetAdjacentNode(bool right) const
	{
		if (_top == nullptr)
			return nullptr;

		for (size_t i = 0; i < _top->_subnodes.size(); i++)
		{
			if (_top->_subnodes[i]->Ptr() == this)
			{
				if (!right)
				{
					if (i == 0)
						return nullptr;
					else
						return _top->_subnodes[i - 1]->Ptr();
				}
				else
				{
					if (i == _top->_subnodes.size() - 1)
						return nullptr;
					else
						return _top->_subnodes[i + 1]->Ptr();
				}
			}
		}

		return nullptr;
	}

	Node* Node::LastSubnode()
	{
		if (_subnodes.size() == 0)
			return nullptr;
		return _subnodes.back()->Ptr();
	}

	NodePtr Node::AtNode(size_t i)
	{
		if (i >= _subnodes.size())
			throw std::runtime_error("Node::AtNode: Out of subnode's list boundaries.");
		
		_subnodes[i]->_topIndex = i;
		return _subnodes[i];
	}

	void Node::PrintTabs(int substract)
	{
		substract += globalTabMinus;
		for (int i = 0; i < _depthIndex - substract; i++)
			_out << "\t";
	}

	size_t Node::CountSubnodes() const
	{
		return _subnodes.size();
	}

	size_t Node::GetDepth() const
	{
		return _depthIndex;
	}

	bool Node::IsBlock() const
	{
		return _isBlock;
	}

	bool Node::IsComplete() const
	{
		return _isComplete;
	}

	void Node::Complete(bool isComplete)
	{
		_isComplete = isComplete;
	}

	bool Node::IsString() const
	{
		return _isString;
	}

	bool Node::IsWrapper() const
	{
		return _isWrapper;
	}

	RootNode::RootNode(OutputInterface& outIf)
		: Node(outIf, Node::Type::RootNode, nullptr)
		, _totalNodes(1)
		, _printedNodes(0)
		, _notifyCallback(nullptr)
		, _useCallback(false)
	{
		_root = this;
	}
	
	void RootNode::ToLua() 
	{
		CountAllNodes();

		for (auto& node : _subnodes)
			node->ToLua();
		TryToCreatePrintNotify();
	}

	void RootNode::InitData(const std::vector<std::string>& strings)
	{
		appLog(Warning) << "Attempting to initialize RootNode. Init is not needed.";
		appLog(Debug) << strings;
	}

	void RootNode::TryToCreatePrintNotify()
	{
		_printedNodes++;
		if (_useCallback)
		{
			(*_notifyCallback)(_printedNodes, _totalNodes);
		}
	}

	void RootNode::IncrementNodeCount(size_t i)
	{
		_totalNodes += int(i);
	}

	void RootNode::CountAllNodes()
	{
		IncrementNodeCount(_subnodes.size());
		for (auto& node : _subnodes)
			node->CountAllNodes();
	}

	void RootNode::AddGlobalId(const std::string& id)
	{
		AssertGlobalId(id);
		_globalIdSet.insert(id);
	}

	void RootNode::AssertGlobalId(const std::string& id)
	{
		if (_globalIdSet.size() == 0)
			return;

		if (_globalIdSet.find(id) != _globalIdSet.end())
		{
			appLog(Warning) << "Redefinition of global identifier" << id << OutputInterface::nl << "Please, rename it and try again.";
			throw std::runtime_error("RootNode::AddGlobalId: Failed to register global identifier.");
		}
	}

	void RootNode::AddNotifyCallback(NotifyCallback callback)
	{
		_useCallback = true;
		_notifyCallback = std::shared_ptr<std::function<void(int, int)>>(new std::function<void(int,int)>(callback));
	}

	void RootNode::Clear()
	{
		_tabs = 0;
		_printedNodes = 0;
		_totalNodes = 1;
		_notifyCallback = nullptr;
		_subnodes.clear();
		_strIds.clear();
		_globalIdSet.clear();
		ResetIterator();
	}

	void RootNode::InsertStringId(const std::string& str)
	{
		_strIds.push_back(str);
	}

	bool RootNode::HasStringId(const std::string& strId) const
	{
		for (const auto& str : _strIds)
		{
			if (str == strId)
				return true;
		}
		return false;
	}

	GlobalDeclaration::GlobalDeclaration(OutputInterface& outIf, Node* top)
		: Node(outIf, Node::Type::GlobalDeclaration, top)
		, _varType("")
		, _varName("")
		, _isNative(false)
		, _isArray(false)
	{
		_depthIndex = 1;
	}

	void GlobalDeclaration::CountAllNodes()
	{
		_root->IncrementNodeCount(_subnodes.size());
		for (auto& node : _subnodes)
			node->CountAllNodes();
	}

	void GlobalDeclaration::ToLua()
	{
		bool emmy = Settings::emmyDoc();

		if (_varType == "macro_def")
		{
			Node* deepest = this;

			while (deepest->CountSubnodes() != 0)
				deepest = deepest->AtNode(0)->Ptr();

			if (deepest->GetType() == Node::Type::OperationObject
				&& deepest->Ptr<OperationObject>()->GetOpType() == OperationObject::OpType::Constant)
			{
				auto ctype = _subnodes[0]->AtNode(0)->Ptr<OperationObject>()->GetConstType();
				switch (ctype)
				{
				case OperationObject::ConstType::Bool:
					_varType = "boolean";
					break;

				case OperationObject::ConstType::Float:
					_varType = "real";
					break;

				case OperationObject::ConstType::RawCode:
				case OperationObject::ConstType::Integer:
					_varType = "integer";
					break;

				case OperationObject::ConstType::String:
					_varType = "string";
					_root->InsertStringId(_varName);
					break;
				}
			}
		}

		_out << OutputInterface::nl;

		if (_isArray)
			_out << _varName << " = {}";
		else if (CountSubnodes() > 0)
		{
			_out << _varName << " = ";
			_subnodes[0]->ToLua();
		}
		else
			_out << "-- " << _varName;

		if (emmy)
			_out << "\t" << "---@type " << _varType;

		if (_isNative)
			_out << " (native) ";

		_out << "\t";

		if (_root)
			_root->TryToCreatePrintNotify();
	}

	void GlobalDeclaration::InitData(const std::vector<std::string>& strings)
	{
		size_t size = strings.size();
		if (size != 3 && size != 2)
		{
			appLog(Critical) << "GlobalDeclaration::InitData: Wrong set of input data. Got" << size << ", expected" << "2 or 3";
			appLog(Debug) << "Args:" << strings;
			return;
		}

		_varType = strings[0];
		_varName = strings[1];
		if (_varName == "end")
			_varName = "end_";

		_root->AssertGlobalId(_varName);

		if (size == 3 && strings[2].find("native") != std::string::npos)
			_isNative = true;

		if (size == 3 && strings[2].find("array") != std::string::npos)
			_isArray = true;
	}

	Comment::Comment(OutputInterface& outIf, Node* top)
		: Node(outIf, Node::Type::Comment, top)
		, _comment("")
	{
	}

	void Comment::ToLua()
	{
		size_t end = _comment.length() - 1;
		bool multiline = false;
		size_t n = std::string::npos, r = std::string::npos;
		std::string tmp = _comment;
		bool rootComment = Top()->GetType() == Node::Type::RootNode;
		if (_comment.length() >= 2)
		{
			tmp = reu::IndexSubstr(_comment, 1, end);
			if (end != 0)
				r = (_comment[0] == '\r' ? 0 : std::string::npos);
			n = tmp.find("\n");
			multiline = (((r != std::string::npos) || (n != std::string::npos)) && (r != 0 && n != 0));
		}
		n = _comment.find("\n");

		if (n == 0 && end == 0)
		{
			if (_root)
				_root->TryToCreatePrintNotify();
			return;
		}

		if (r == 0)
		{
			_comment = _comment.substr(2, _comment.length() - 2);
			_out << OutputInterface::nl;

			if (!rootComment)
				PrintTabs();
		}
		else if (n == 0)
		{
			_comment = _comment.substr(1, _comment.length() - 1);
			_out << OutputInterface::nl;

			if (!rootComment)
				PrintTabs();
		}
		else if (!multiline && !rootComment)
			_out << "\t";

		tmp = _comment;
		
		if (multiline)
			_out << OutputInterface::nl << "--[[" << OutputInterface::nl << tmp << OutputInterface::nl << "]]--" << OutputInterface::nl;
		else
			_out << "-- " << tmp;

		if (_root)
			_root->TryToCreatePrintNotify();
	}

	void Comment::CountAllNodes()
	{
	}

	void Comment::InitData(const std::vector<std::string>& strings)
	{
		size_t size = strings.size();
		if (size != 1)
		{
			appLog(Critical) << "Comment::InitData: Wrong set of input data. Got" << size << ", expected" << "1";
			appLog(Debug) << "Args:" << strings;
			return;
		}

		_comment = strings[0];
		size_t last = _comment.size() - 1;
		size_t cutLen = 0;
		if (_comment.length() > 1)
		{
			if (_comment[last] == '\r' || _comment[last] == '\n')
				cutLen++;
			if (last > 0 && _comment[last - 1] == '\r' || _comment[last - 1] == '\n')
				cutLen++;
		}
		if (cutLen > 0)
			_comment = reu::IndexSubstr(_comment, 0, last - cutLen);
	}

	Function::Function(OutputInterface& outIf, Node* top)
		: Node(outIf, Node::Type::Function, top)
		, _name("")
		, _returnType("")
		, _args({})
		, _isNative(false)
	{
		_isBlock = true;
	}

	void Function::ToLua()
	{
		bool emmy = Settings::emmyDoc();

		if (emmy)
		{
			if (_args.size() > 0)
				_out << OutputInterface::nl;
			std::string tmp;
			for (size_t i = 0; i < _args.size(); i++)
			{
				if ((i+1) % 2 != 0)
				{
					tmp = _args[i];
					continue;
				}

				if ((i + 1) % 2 == 0)
					_out << OutputInterface::nl << "---@param " << _args[i] << " " << tmp;
			}
			_out << OutputInterface::nl << "---@return " << _returnType;
		}
		
		_out << OutputInterface::nl << "function " << _name << "(";
		
		if (_args.size() == 0)
			_out << ")";
		else if (_args.size() == 2)
			_out << _args[1] << ")";
		else
		{
			for (size_t i = 1; i < _args.size(); i++)
			{
				if (i % 2 != 0)
				{
					_out << _args[i];
					if (i != _args.size() - 1)
						_out << ", ";
					else
						_out << ")";
				}
			}
		}

		if (!_isNative)
		{
			for (auto& node : _subnodes)
				node->ToLua();
			_out << OutputInterface::nl << "end" << OutputInterface::nl;
		}
		else
			_out << " end\t-- (native)" << OutputInterface::nl;
		

		if (_root)
			_root->TryToCreatePrintNotify();
	}

	void Function::CountAllNodes()
	{
		_root->IncrementNodeCount(_subnodes.size());
		for (auto& node : _subnodes)
			node->CountAllNodes();
	}

	void Function::InitData(const std::vector<std::string>& strings)
	{
		size_t size = strings.size();
		if (size < 2)
		{
			appLog(Critical) << "Function::InitData: Wrong set of input data. Got" << size << ", expected" << "2";
			appLog(Debug) << "Args:" << strings;
			return;
		}

		_returnType = strings[0];
		if (_returnType[0] == '!')
		{
			_returnType = _returnType.substr(1, _returnType.length() - 1);
			_isNative = true;
		}
		_name = strings[1];
		_depthIndex++;
		bool insStr = false;

		std::string tmp;
		for (size_t i = 2; i < size; i++)
		{
			tmp = strings[i];
			if (tmp == "end")
				tmp = "end_";
			_args.push_back(tmp);
			if (i % 2 == 0 && tmp == "string")
				insStr = true;
			if (i % 2 != 0 && insStr)
			{
				_strIds.push_back(tmp);
				insStr = false;
			}
		}
			
	}

	void Function::InsertStringId(const std::string& strId)
	{
		_strIds.push_back(strId);
	}

	bool Function::HasStringId(const std::string& strId) const
	{
		for (const auto& str : _strIds)
		{
			if (str == strId)
				return true;
		}
		return false;
	}

	bool OperationObject::CheckIsString()
	{
		auto top = _top;
		while (top->Top() != nullptr)
			top = top->Top();
		if (top->Ptr<RootNode>()->HasStringId(_opText))
			return true;
		else
		{
			top = Top();
			while (top->Top() != nullptr && top->Top()->GetType() == Node::Type::Function)
				top = top->Top();
			if (top->GetType() == Node::Type::Function)
			{
				if (top->Ptr<Function>()->HasStringId(_opText))
					return true;
			}
		}

		return false;
	}

	OperationObject::OperationObject(OutputInterface& outIf, Node* top)
		: Node(outIf, Node::Type::OperationObject, top)
		, _opText("")
		, _extra("")
		, _otype(OpType::Unknown)
		, _inBrackets(false)
		, _unaryExpr(false)
		, _lambdaIsSingle(false)
		, _blockClosed(false)
		, _cType(ConstType::Undefined)
		, _AssignUnary(false)
	{
	}

	std::string OperationObject::GetFullIdName()
	{
		std::string name = _opText;
		Node::Type mainTopNodeType;

		Node* mainNode = this;
		do 
		{
			mainNode = mainNode->Top();
			if (mainNode == nullptr)
				return name;
		} 
		while (mainNode->GetType() != Node::Type::Function && mainNode->GetType() != Node::Type::Method);

		mainTopNodeType = mainNode->GetType();

		if (mainTopNodeType == Node::Type::Function)
			return name;

		auto classNode = mainNode->Top();

		if (mainNode->Ptr<Method>()->IsStatic())
		{
			if (mainNode->Ptr<Method>()->GetReplaceThisName() != "" && mainNode->Ptr<Method>()->GetReplaceThisName() == name)
				return "this";
			return name;
		}	

		if (classNode->Ptr<Class>()->HasNonstaticMember(name))
		{
			Node* top = _top;
			size_t this_index = 9999999;
			std::vector<std::string> leftNeighbours;

			if (top->CountSubnodes() < 3)
				return "this." + name;

			for (size_t i = 0; i < top->CountSubnodes(); i++)
			{
				auto node = top->AtNode(i);

				if (i >= this_index)
					break;

				if (node->Ptr() == dynamic_cast<Node*>(this) && this_index == 9999999)
				{
					this_index = i;
					if (i < 2)
						return "this." + name;
					i-= 2;
				}

				if (i == this_index - 1)
				{
					node = top->AtNode(i);
					leftNeighbours.push_back(node->Ptr<OperationObject>()->_opText);
				}

				if (i == this_index - 2)
				{
					node = top->AtNode(i);
					leftNeighbours.push_back(node->Ptr<OperationObject>()->_opText);
				}
			}

			if (leftNeighbours[1] != ".")
				name = "this." + name;
		}

		return name;
	}

	void OperationObject::ToLua()
	{
		size_t nodeCount, lastIndex;
		bool begin = false;
		bool allowNewLines = false;
		size_t stopIndex = 0;
		std::string stmp;
		int itmp;

		if (_otype != OpType::Unknown)
		{
			if (_isNewLine)
			{
				_out << OutputInterface::nl;
				PrintTabs();
			}
		}

		switch (_otype)
		{
		case OpType::Id:
			_out << GetFullIdName();
			for (auto& node : _subnodes)
				node->ToLua();
			break;

		case OpType::Constant:
			_out << Utils::const2lua(_opText);
			g_prevConst = _cType;
			break;

		case OpType::Operator:
			_out << Utils::op2lua(_opText, HasNeighbourStrings());
			break;

		case OpType::UnaryOperator:
			if (!_unaryExpr)
			{
				if (_extra == "++")
				{
					if (_AssignUnary)
						_out << GetFullIdName() << " = " << GetFullIdName() << " + 1";
					else
					{
						_out << "(" << GetFullIdName() << " + 1" << ")";
					}
				}
				else if (_extra == "--")
				{
					if (_AssignUnary)
						_out << GetFullIdName() << " = " << GetFullIdName() << " - 1";
					else
						_out << "(" << GetFullIdName() << " - 1" << ")";
				}
					
			}
			else
			{
				if (!_isString)
					_out << GetFullIdName() << " = " << GetFullIdName() << std::string({ ' ', _extra[0], ' ' });
				else
					_out << GetFullIdName() << " = " << GetFullIdName() << " .. ";
				for (auto& node : _subnodes)
					node->ToLua();
			}
			break;

		case OpType::Argument:
			for (auto& node : _subnodes)
				node->ToLua();
			break;

		case OpType::VarInitExpression:
		case OpType::Expression:
			if (_inBrackets)
				_out << "(";

			for (auto& node : _subnodes)
				node->ToLua();

			if (_inBrackets)
				_out << ")";
			break;

		case OpType::Call:
			_out << GetFullIdName() << "(";
			nodeCount = CountSubnodes();
			if (nodeCount > 0)
			{
				lastIndex = nodeCount - 1;
				for (size_t i = 0; i <= lastIndex; i++)
				{
					auto node = AtNode(i);
					node->ToLua();
					if (i != lastIndex)
						_out << ", ";
				}
			}
			_out << ")";
			break;

		case OpType::Wrapper:
			_out << OutputInterface::nl;
			PrintTabs();
			if (_inBrackets)
				_out << "(";

			for (auto& node : _subnodes)
				node->ToLua();

			if (_inBrackets)
				_out << ")";
			break;

		case OpType::GVarInitWrapper:
			for (auto& node : _subnodes)
				node->ToLua();
			break;

		case OpType::Return:
			_out << OutputInterface::nl;
			PrintTabs();
			_out << "return ";
			for (auto& node : _subnodes)
				node->ToLua();
			break;

		case OpType::DoStatement:
			_out << "until ";
			if (_extra == "not")
				_out << "not( ";
			for (auto& node : _subnodes)
				node->ToLua();
			if (_extra == "not")
				_out << " )";
			break;

		case OpType::ExitWhen:
			_out << OutputInterface::nl;
			PrintTabs();
			_out << "if ";
			for (auto& node : _subnodes)
				node->ToLua();
			_out << " then break end";
			break;

		case OpType::Index:
			_out << "[";
			if (_subnodes.size() == 1 
				&& _subnodes[0]->GetType() == Node::Type::OperationObject
				&& _subnodes[0]->Ptr<OperationObject>()->GetOpType() == OperationObject::OpType::Constant
				&& reu::IsMatching(_subnodes[0]->Ptr<OperationObject>()->_opText, "^[0-9]+$")
				&& !Settings::DoNotIncrementArrayIndexConstants)
			{
				itmp = atoi(_subnodes[0]->Ptr<OperationObject>()->_opText.c_str()) + 1;
				_subnodes[0]->Ptr<OperationObject>()->_opText = std::to_string(itmp);
				_subnodes[0]->ToLua();
			}
			else
			{
				for (auto& node : _subnodes)
					node->ToLua();
			}
			_out << "]";
			break;

		case OpType::Logic:
			for (size_t i = 0; i < CountSubnodes(); i++)
			{
				auto node = AtNode(i);
				auto prevNode = (i == 0) ? node : AtNode(i - 1);
				bool isNewLine = (node->GetType() == Node::Type::OperationObject
					&& node->Ptr<OperationObject>()->GetOpType() == OperationObject::OpType::NewLine);
				bool isNewLinePrev = (prevNode->GetType() == Node::Type::OperationObject
					&& prevNode->Ptr<OperationObject>()->GetOpType() == OperationObject::OpType::NewLine);

				if ((isNewLine && isNewLinePrev) || (isNewLine && i == CountSubnodes() - 1))
				{
					_out << OutputInterface::nl;
					PrintTabs();
					_out << "end";
					allowNewLines = true;
					stopIndex = i;
					break;
				}

				if (!isNewLine)
					node->ToLua();
			}

			if (allowNewLines)
			{
				for (size_t i = stopIndex; i < CountSubnodes(); i++)
				{
					auto node = AtNode(i);
					node->ToLua();
				}
			}
			else
			{
				_out << OutputInterface::nl;
				PrintTabs();
				_out << "end";
			}
				
			break;

		case OpType::If:
			_out << OutputInterface::nl;
			PrintTabs(1);
			_out << "if ";
			if (CountSubnodes() == 0)
				appLog(Warning) << "'if' without statement!";
			for (size_t i = 0; i < CountSubnodes(); i++)
			{
				auto node = AtNode(i);
				node->ToLua();
				if (i == 0)
					_out << " then";
			}
			break;

		case OpType::Elseif:
			_out << OutputInterface::nl;
			PrintTabs(1);
			_out << "elseif ";
			if (CountSubnodes() == 0)
				appLog(Warning) << "'elseif' without statement!";
			for (size_t i = 0; i < CountSubnodes(); i++)
			{
				auto node = AtNode(i);
				node->ToLua();
				if (i == 0)
					_out << " then";
			}
			break;

		case OpType::Else:
			_out << OutputInterface::nl;
			PrintTabs(1);
			_out << "else";
			for (auto& node : _subnodes)
				node->ToLua();
			break;

		case OpType::Loop:
			_out << OutputInterface::nl;
			PrintTabs(1);
			_out << "while true do";
			for (auto& node : _subnodes)
				node->ToLua();
			_out << OutputInterface::nl;
			PrintTabs(1);
			_out << "end";
			break;

		case OpType::Do:
			_out << OutputInterface::nl;
			PrintTabs(1);
			_out << "repeat";
			for (auto& node : _subnodes)
				node->ToLua();
			_out << OutputInterface::nl;
			PrintTabs(1);
			break;

		case OpType::For:
			break;

		case OpType::While:
			_out << OutputInterface::nl;
			PrintTabs(1);
			_out << "while ";
			if (CountSubnodes() == 0)
				appLog(Warning) << "'while' without statement!";
			for (size_t i = 0; i < CountSubnodes(); i++)
			{
				auto node = AtNode(i);
				node->ToLua();
				if (i == 0)
					_out << " do";
			}
			_out << OutputInterface::nl;
			PrintTabs(1);
			_out << "end";
			break;

		case OpType::WhileNot:
			_out << OutputInterface::nl;
			PrintTabs(1);
			_out << "while not ( ";
			if (CountSubnodes() == 0)
				appLog(Warning) << "'whilenot' without statement!";
			for (size_t i = 0; i < CountSubnodes(); i++)
			{
				auto node = AtNode(i);
				node->ToLua();
				if (i == 0)
					_out << " ) do";
			}
			_out << OutputInterface::nl;
			PrintTabs(1);
			_out << "end";
			break;

		case OpType::Lambda:
			_out << "function ()";
			
			for (auto& node : _subnodes)
				node->ToLua();
				
			_out << OutputInterface::nl;
			PrintTabs(1);
			_out << "end";
			break;

		case OpType::NewLine:
			_out << OutputInterface::nl;
			if (Top()->GetType() != Node::Type::RootNode)
				PrintTabs();
			break;

		case OpType::empty:	
			break;
		
		default:
			appLog(Warning) << "OperationObject::ToLua: Unsupported OpType.";
		}

		if (_root)
			_root->TryToCreatePrintNotify();
	}

	void OperationObject::CountAllNodes()
	{
		_root->IncrementNodeCount(_subnodes.size());
		for (auto& node : _subnodes)
			node->CountAllNodes();
	}

	void OperationObject::InitData(const std::vector<std::string>& strings)
	{
		size_t size = strings.size();
		Node* top = _top;
		Method* met;
		
		if (size == 0 || size > 3)
		{
			appLog(Critical) << "OperationObject::InitData: Wrong set of input data. Got" << size << ", expected" << "3";
			appLog(Debug) << "Args:" << strings;
			return;
		}

		auto& s = strings[0];

		if (s.length() < 1 || s.length() > 2)
			throw std::runtime_error("OperationObject::InitData: wrong key length.");

		switch (s[0])
		{
		case 'c':
			_otype = OpType::Call;
			_opText = strings[1];
			break;

		case 'C':
			_otype = OpType::Constant;
			_opText = strings[1];
			_cType = Utils::determConstType(_opText);
			if (_cType == ConstType::String)
				_isString = true;
			
			break;

		case 'I':
			_otype = OpType::Id;
			_opText = strings[1];
			if (_opText == "end")
				_opText = "end_";
			_isString = CheckIsString();
			break;

		case 'x':
			_otype = OpType::Expression;
			if (s[1] == 'b')
				_inBrackets = true;
			if (Top()->GetType() == Node::Type::OperationObject &&
				(Top()->Ptr<OperationObject>()->GetOpType() == OpType::If
					|| Top()->Ptr<OperationObject>()->GetOpType() == OpType::Elseif
					|| Top()->Ptr<OperationObject>()->GetOpType() == OpType::While)
				&& Top()->CountSubnodes() <= 1)
			{
				_inBrackets = false;
			}
			break;

		case 'v':
			_otype = OpType::VarInitExpression;
			_isWrapper = true;
			_opText = strings[1];
			break;

		case 'a':
			_otype = OpType::Argument;
			_isWrapper = true;
			break;

		case 'S':
			_otype = OpType::DoStatement;
			if (s[1] == 'n')
				_extra = "not";
			_isWrapper = true;
			break;

		case 'f':
			_otype = OpType::For;
			break;

		case 'r':
			if (top->GetType() == Node::Type::Method)
			{
				met = top->Ptr<Method>();
				if (met->GetMethodName() == "create" || met->GetMethodName() == "new")
				{
					_otype = OpType::empty;
					break;
				}
			}
			_otype = OpType::Return;
			_isWrapper = true;
			break;

		case 'T':
			_otype = OpType::ExitWhen;
			_isWrapper = true;
			break;

		case 'L':
			_otype = OpType::Logic;
			break;

		case 'i':
			_otype = OpType::If;
			_depthIndex++;
			_isBlock = true;
			break;

		case 'E':
			_otype = OpType::Elseif;
			_depthIndex++;
			_isBlock = true;
			break;

		case 'e':
			_otype = OpType::Else;
			_depthIndex++;
			_isBlock = true;
			break;

		case 'P':
			_otype = OpType::Loop;
			_depthIndex++;
			_isBlock = true;
			break;

		case 'D':
			_otype = OpType::Do;
			_depthIndex++;
			_isBlock = true;
			break;

		case 'w':
			if (s[1] == 'n')
				_otype = OpType::WhileNot;
			else
				_otype = OpType::While;
			
			_depthIndex++;
			_isBlock = true;
			break;

		case 'l':
			if (s[1] == 's')
				_lambdaIsSingle = true;
			_otype = OpType::Lambda;
			_opText = strings[1];
			_depthIndex++;
			_isBlock = true;
			break;

		case 'o':
			_otype = OpType::Operator;
			_opText = strings[1];
			break;

		case 'u':
			_otype = OpType::UnaryOperator;
			_opText = strings[1];
			_extra = strings[2];
			if (s[1] == 'e')
				_unaryExpr = true;
			else if (s[1] == 'a')
				_AssignUnary = true;
			_isString = CheckIsString();
			break;

		case 'g':
			_otype = OpType::GVarInitWrapper;
			_isWrapper = true;
			break;
		case 'W':
			_otype = OpType::Wrapper; 
			_isWrapper = true;
			break;

		case 'n':
			_otype = OpType::Index;
			break;

		case 'N':
			_otype = OpType::NewLine;
			break;

		case '_':
			_otype = OpType::empty;
			_opText = "";
			_isWrapper = false;
			break;

		default:
			appLog(Warning) << "OperationObject::InitData: unknown key.";
		}

	}

	bool OperationObject::HasNeighbourStrings()
	{
		Node* top = _top;
		size_t this_index = 9999999;

		if (top->IsString())
			return true;

		std::vector<std::string> neighbours;
		for (size_t i = 0; i < top->CountSubnodes(); i++)
		{
			auto node = top->AtNode(i);

			if (i > this_index + 1)
				break;

			if (node->Ptr() == dynamic_cast<Node*>(this) && this_index == 9999999)
			{
				this_index = i;
				if (i > 0)
					i--;
			}

			if (i == this_index - 1)
			{
				node = top->AtNode(i);
				if (node->IsString())
					return true;

				if (node->Ptr<OperationObject>()->GetOpType() == OperationObject::OpType::Index)
				{
					for (int j = int(i); j >= 0; j--)
					{
						node = top->AtNode(j);
						if (node->IsString())
							return true;

						if (node->Ptr<OperationObject>()->GetOpType() != OperationObject::OpType::Index)
						{
							neighbours.push_back(node->Ptr<OperationObject>()->_opText);
							break;
						}
					}
				}
				else
					neighbours.push_back(node->Ptr<OperationObject>()->_opText);
			}

			if (i == this_index + 1)
			{
				node = top->AtNode(i);
				if (node->IsString())
					return true;
				neighbours.push_back(node->Ptr<OperationObject>()->_opText);
			}
		}

		Function* fnode = nullptr;
		RootNode* gspace = nullptr;

		top = this;
		while (top != nullptr)
		{
			if (top->GetType() == Node::Type::Function)
			{
				fnode = top->Ptr<Function>();
				break;
			}
			top = top->Top();
		}

		top = _top;
		while (top != nullptr)
		{
			if (top->Top() == nullptr)
			{
				gspace = top->Ptr<RootNode>();
				break;
			}
			top = top->Top();
		}

		if (fnode != nullptr)
		{
			for (auto n : neighbours)
			{
				if (n != "" && fnode->HasStringId(n))
					return true;
			}
		}

		if (gspace != nullptr)
		{
			for (auto n : neighbours)
			{
				if (n != "" && gspace->HasStringId(n))
					return true;
			}
		}
		else
			appLog(Critical) << "OperationObject::HasNeighbourStrings: Root node not found.";

		return false;
	}

	OperationObject::ConstType OperationObject::GetConstType() const
	{
		return _cType;
	}

	std::string OperationObject::GetText() const
	{
		return _opText;
	}

	OperationObject::OpType OperationObject::GetOpType() const
	{
		return _otype;
	}

	bool OperationObject::isEmpty() const
	{
		return (_opText == "");
	}

	bool OperationObject::LambdaIsSingle() const
	{
		return _lambdaIsSingle;
	}

	bool OperationObject::BlockClosed() const
	{
		return _blockClosed;
	}

	void OperationObject::CloseBlock()
	{
		_blockClosed = true;
	}

	LocalDeclaration::LocalDeclaration(OutputInterface& outIf, Node* top)
		: Node(outIf, Node::Type::LocalDeclaration, top)
		, _type("")
		, _vars({})
		, _arrSizes({})
		, _doNotPrint(false)
	{
	}

	void LocalDeclaration::ToLua()
	{
		if (_doNotPrint)
		{
			if (_root)
				_root->TryToCreatePrintNotify();
			return;
		}

		_out << OutputInterface::nl;
		PrintTabs();
		_out << "local ";

		//Group vars with init to begin
		std::vector<std::string>		   vv;
		std::vector<std::string>		   av;
		std::vector<std::shared_ptr<Node>> nv;
		for (size_t i = 0; i < _vars.size(); i++)
		{
			if (AtNode(i)->CountSubnodes() != 0 || _arrSizes[i] != "")
			{

				auto v = _vars[i];
				auto a = _arrSizes[i];
				auto n = AtNode(i);

				vv.push_back(v);
				av.push_back(a);
				nv.push_back(n);
			}
		}

		for (size_t i = 0; i < _vars.size(); i++)
		{
			if (AtNode(i)->CountSubnodes() == 0 && _arrSizes[i] == "")
			{

				auto v = _vars[i];
				auto a = _arrSizes[i];
				auto n = AtNode(i);

				vv.push_back(v);
				av.push_back(a);
				nv.push_back(n);
			}
		}

		_vars = vv;
		_arrSizes = av;
		_subnodes = nv;

		for (size_t i = 0; i < _vars.size(); i++)
		{
			_out << _vars[i];
			auto expr = AtNode(0);

			if (i != _vars.size() - 1)
				_out << ", ";
			else if (_arrSizes[0] != "" || expr->CountSubnodes() > 0)
			{
				_out << " = ";

				for (size_t j = 0; j < _vars.size(); j++)
				{
					expr = AtNode(j);
					if (_arrSizes[j] == "" && expr->CountSubnodes() == 0)
						continue;
					else if (j != 0)
						_out << ", ";

					if (_arrSizes[j] != "")
						_out << "{}";
					else if (expr->CountSubnodes() > 0)
						expr->ToLua();
				}
			}
		}

		if (_root)
			_root->TryToCreatePrintNotify();
	}

	void LocalDeclaration::CountAllNodes()
	{
		_root->IncrementNodeCount(_subnodes.size());
		for (auto& node : _subnodes)
			node->CountAllNodes();
	}

	void LocalDeclaration::InitData(const std::vector<std::string>& strings)
	{
		appLog(Debug) << "LocalDeclaration::InitData Args:" << strings;
		_type == strings[0];
		if (_type == "string")
			_isString = true;
	}

	size_t LocalDeclaration::GetVarCount() const
	{
		return _vars.size();
	}

	bool LocalDeclaration::hasType() const
	{
		return (_type != "");
	}

	void LocalDeclaration::DoNotPrint()
	{
		_doNotPrint = true;
	}

	Node* LocalDeclaration::AddVariable(const std::string& name, const std::string& arrSize)
	{
		appLog(Debug) << "Adding local variable" << name;
		_root->AssertGlobalId(name);
		if (name == "end")
			_vars.push_back(name + "_");
		else
			_vars.push_back(name);
		_arrSizes.push_back(arrSize);
		auto node = Node::Produce(Node::Type::OperationObject, this, _out);
		node->InitData({ "v", name });
		AddSubnode(node);
		return node->Ptr();
	}

	TypeDef::TypeDef(OutputInterface& outIf, Node* top) 
		: Node(outIf, Node::Type::TypeDef, top)
		, _nativeName("")
		, _extends("")
	{
	}

	void TypeDef::CountAllNodes()
	{
	}

	void TypeDef::ToLua()
	{
		if (!Settings::emmyDoc())
			return;

		_out << OutputInterface::nl << "---@class  " << _nativeName;
		if (_extends != "")
			_out << ":" << _extends << " ";
	}

	void TypeDef::InitData(const std::vector<std::string>& strings)
	{
		size_t size = strings.size();
		if (size != 2)
		{
			appLog(Critical) << "TypeDef::InitData: Wrong set of input data. Got" << size << ", expected" << "2";
			appLog(Debug) << "Args:" << strings;
			return;
		}

		_nativeName = strings[0];
		_extends = strings[1];
	}

	Class::Class(OutputInterface& outIf, Node* top)
		: Node(outIf, Node::Type::Class, top)
		, _className("")
	{
		_depthIndex++;
		_memberList.push_back("deallocate");
		_memberList.push_back("destroy");
	}

	void Class::CountAllNodes()
	{
		_root->IncrementNodeCount(_subnodes.size());
		for (auto& node : _subnodes)
			node->CountAllNodes();
	}

	void Class::ToLua()
	{
		bool emmy = Settings::emmyDoc();
		NodeList staticMethods;
		NodePtr  creationNode = nullptr;
		bool onlyMembers = false;

		for (NodeList::iterator node = _subnodes.begin(); node != _subnodes.end(); node++)
		{
			if ((*node)->GetType() == Node::Type::Method
				&& (*node)->Ptr<Method>()->_name == "new" || (*node)->Ptr<Method>()->_name == "create")
			{
				creationNode = (*node);
				std::swap(_subnodes[0], *node);
				break;
			}
		}

		onlyMembers = (creationNode == nullptr);

		std::string createName = "create";
		if (!onlyMembers)
			createName = creationNode->Ptr<Method>()->_name;

		_out << OutputInterface::nl;
		
		//Declare a class
		if (emmy)
			_out << "---@class  " << _className << OutputInterface::nl;
		_out << _className << " = {}";
		_out << OutputInterface::nl;

		//Declare static members
		for (auto& mem : _staticMembers)
		{
			if (mem.initValue == "" && !mem.isArray)
				_out << "-- " << _className << ":" << mem.name;
			else if (mem.isArray)
				_out << mem.name << " = {}";
			else
				_out << mem.name << " = " << mem.initValue;

			if (emmy)
				_out << "\t" << "---@type " << mem.type;

		}
		_out << OutputInterface::nl;

		if (emmy)
		{
			std::string tmp;
			if (!onlyMembers)
			{
				for (size_t i = 0; i < creationNode->Ptr<Method>()->_args.size(); i++)
				{
					if ((i + 1) % 2 != 0)
					{
						tmp = creationNode->Ptr<Method>()->_args[i];
						continue;
					}

					if ((i + 1) % 2 == 0)
						_out << OutputInterface::nl << "---@param " << creationNode->Ptr<Method>()->_args[i] << " " << tmp;
				}
			}
			_out << OutputInterface::nl << "---@return " << _className << OutputInterface::nl;
		}

		//Print class header
		_out << "function " << _className << ":" << createName << "(";

		if (!onlyMembers)
		{
			//Print .create/.new args
			size_t argsSize = creationNode->Ptr<Method>()->_args.size();
			for (size_t i = 0; i < argsSize; i++)
			{
				if ((i + 1) % 2 == 0)
				{
					auto args = creationNode->Ptr<Method>()->_args[i];
					_out << creationNode->Ptr<Method>()->_args[i];
					if (i != argsSize - 1)
						_out << ", ";
				}
			}
		}
		_out << ")" << OutputInterface::nl;

		//Print this object creation
		PrintTabs();
		_out << "local this = {}";

		//Print non-static members
		for (auto& mem : _members)
		{
			_out << OutputInterface::nl;
			PrintTabs();
			if (mem.initValue == "" && !mem.isArray)
				_out << "-- this." << mem.name;
			else if (mem.isArray)
				_out << "this." << mem.name << " = {}";
			else
				_out << "this." << mem.name << " = " << mem.initValue;

			if (emmy)
				_out << "\t" << "---@type " << mem.type;

		}
		_out << OutputInterface::nl;
		_out << OutputInterface::nl;

		PrintTabs();
		_out << "function this.deallocate() this = nil end" << OutputInterface::nl;

		if (CountSubnodes() > 1)
		{
			//Print non-static methods
			for (size_t i = 1; i < CountSubnodes(); i++)
			{
				auto node = _subnodes[i];

				if (node->Ptr<Method>()->isStatic())
				{
					staticMethods.push_back(node);
					continue;
				}

				node->ToLua();
			}
		}

		_out << OutputInterface::nl;

		if (!onlyMembers)
		{
			//Print .create/.new method subnodes
			globalTabMinus = 1;
			for (auto& node : creationNode->Ptr<Method>()->_subnodes)
				node->ToLua();
			globalTabMinus = 0;
			_out << OutputInterface::nl;
		}

		//Print magic meta-table calls
		PrintTabs();
		_out << "setmetatable(this, self)" << OutputInterface::nl; PrintTabs();
		_out << "self.__index = self" << OutputInterface::nl; PrintTabs();
		_out << "return this" << OutputInterface::nl << "end" << OutputInterface::nl;

		if (CountSubnodes() > 1)
		{
			//Print static methods
			_out << OutputInterface::nl;
			for (auto& node : staticMethods)
				node->ToLua();
		}

		if (_root)
			_root->TryToCreatePrintNotify();
	}

	void Class::InitData(const std::vector<std::string>& strings)
	{
		size_t size = strings.size();
		if (size != 1)
		{
			appLog(Critical) << "TypeDef::InitData: Wrong set of input data. Got" << size << ", expected" << "1";
			appLog(Debug) << "Args:" << strings;
			return;
		}

		_className = strings[0];
	}

	void Class::RegMember(const std::vector<std::string>& mem, bool isStatic)
	{
		size_t size = mem.size();
		if (size != 4)
		{
			appLog(Critical) << " Class::RegMember: Wrong set of input data. Got" << size << ", expected" << "4";
			appLog(Debug) << "Args:" << mem;
			return;
		}

		_root->AssertGlobalId( mem[2] );

		if (mem[2] == "string")
			_strIds.push_back(mem[2]);

		if (isStatic)
			_staticMembers.push_back({ mem[1], mem[2], mem[3], mem[0] == "array" });
		else
		{
			_members.push_back({ mem[1], mem[2], mem[3], mem[0] == "array" });
			_memberList.push_back( mem[2] );
		}
	}

	Node* Class::RegMethod(const std::vector<std::string>& met, bool isStatic)
	{
		size_t size = met.size();
		if (size < 2)
		{
			appLog(Critical) << " Class::RegMethod: Wrong set of input data. Got" << size << ", expected" << "2 or more";
			appLog(Debug) << "Args:" << met;
			return nullptr;
		}

		std::vector<std::string> args;
		std::string static_ = (isStatic ? "static" : "");
		std::string name = met[1];

		if (met[0] == "string")
			_strIds.push_back(name);

		_root->AssertGlobalId(name);
		if (!isStatic)
			_memberList.push_back(name);

		args.push_back(static_);
		for (auto& s : met)
			args.push_back(s);

		auto node = Node::Produce(Node::Type::Method, this, _out);
		AddSubnode(node);
		node->InitData(args);

		return node->Ptr();
	}

	std::string Class::GetClassName() const
	{
		return _className;
	}

	bool Class::HasNonstaticMember(const std::string& name) const
	{
		for (auto& m : _memberList)
		{
			if (m == name)
				return true;
		}

		return false;
	}

	Method::Method(OutputInterface& outIf, Node* top)
		: Node(outIf, Node::Type::Method, top)
		, _name("")
		, _returnType("")
		, _isStatic(false)
	{
		_depthIndex = 0;
	}

	void Method::CountAllNodes()
	{
		_root->IncrementNodeCount(_subnodes.size());
		for (auto& node : _subnodes)
			node->CountAllNodes();
	}

	bool Method::isStatic() const
	{
		return _isStatic;
	}

	void Method::InsertStringId(const std::string& strId)
	{
		_strIds.push_back(strId);
	}

	bool Method::HasStringId(const std::string& strId) const
	{
		for (const auto& str : _strIds)
		{
			if (str == strId)
				return true;
		}
		return false;
	}

	void Method::ToLua()
	{
		bool emmy = Settings::emmyDoc();
		std::string className = _top->Ptr<Class>()->GetClassName();

		if (emmy)
		{
			if (_args.size() > 0)
				_out << OutputInterface::nl;
			std::string tmp;
			for (size_t i = 0; i < _args.size(); i++)
			{
				if ((i + 1) % 2 != 0)
				{
					tmp = _args[i];
					continue;
				}

				if ((i + 1) % 2 == 0)
				{
					if (!_isStatic)
					{
						_out << OutputInterface::nl;
						PrintTabs(1);
						_out << "---@param " << _args[i] << " " << tmp;
					}
					else
						_out << OutputInterface::nl << "---@param " << _args[i] << " " << tmp;
				}
			}
			if (!_isStatic)
			{
				_out << OutputInterface::nl;
				PrintTabs(1);
				_out << "---@return " << _returnType;
			}
			else
				_out << OutputInterface::nl << "---@return " << _returnType;
		}

		if (!_isStatic)
		{
			_out << OutputInterface::nl;
			PrintTabs(1);
			_out << "function " << "this." << _name << "(";
		}
		else
			_out << "function " << className << ":" << _name << "(";
			

		if (_args.size() == 0)
			_out << ")";
		else if (_args.size() == 2)
			_out << _args[1] << ")";
		else
		{
			for (size_t i = 1; i < _args.size(); i++)
			{
				if (i % 2 != 0)
				{
					_out << _args[i];
					if (i != _args.size() - 1)
						_out << ", ";
					else
						_out << ")";
				}
			}
		}

		for (auto& node : _subnodes)
			node->ToLua();
		if (!_isStatic)
		{
			_out << OutputInterface::nl;
			PrintTabs(1);
			_out << "end" << OutputInterface::nl;
		}
		else
			_out << OutputInterface::nl << "end" << OutputInterface::nl;
		
		if (_root)
			_root->TryToCreatePrintNotify();
	}

	bool Method::IsStatic() const
	{
		return _isStatic;
	}

	std::string Method::GetMethodName() const
	{
		return _name;
	}

	void Method::SetThisname(const std::string& name)
	{
		_thisName = name;
	}

	std::string Method::GetReplaceThisName() const
	{
		return _thisName;
	}

	void Method::InitData(const std::vector<std::string>& strings)
	{
		size_t size = strings.size();
		if (size < 3)
		{
			appLog(Critical) << " Class::RegMember: Wrong set of input data. Got" << size << ", expected" << "3 or more";
			appLog(Debug) << "Args:" << strings;
			return;
		}

		if (_top->GetType() != Node::Type::Class)
			throw std::runtime_error("Method::InitData: Error, top node must be class. Wrong declaration of method.");

		_root->AssertGlobalId(strings[1]);

		_isStatic = strings[0] == "static";
		_returnType = strings[1];
		_name = strings[2];

		bool insStr = false;

		for (auto& strid : Top()->Ptr<Class>()->_strIds)
			_strIds.push_back(strid);

		std::string tmp;
		for (size_t i = 3; i < size; i++)
		{
			tmp = strings[i];
			if (tmp == "end")
				tmp = "end_";
			_args.push_back(tmp);
			if (i % 2 != 0 && tmp == "string")
				insStr = true;
			if (i % 2 == 0 && insStr)
			{
				_strIds.push_back(tmp);
				insStr = false;
			}
		}
	}

} //namespace cJass