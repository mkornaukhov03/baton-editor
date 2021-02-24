#ifndef BATON_ENUMS_H
#define BATON_ENUMS_H

// Enums with magic constants, defined by JSON RPC / LSP / Clangd

enum class ErrorCode {
  ParseError = -32700,
  InvalidRequest = -32600,
  MethodNotFound = -32601,
  InvalidParams = -32602,
  InternalError = -32603,
  ServerNotInitialized = -32002,
  UnknownErrorCode = -32001,
  RequestCancelled = -32800,
  ContentModified = -32801,
};
enum class TraceLevel {
  Off = 0,
  Messages = 1,
  Verbose = 2,
};
enum class TextDocumentSyncKind {
  None = 0,
  Full = 1,
  Incremental = 2,
};
enum class CompletionItemKind {
  Missing = 0,
  Text = 1,
  Method = 2,
  Function = 3,
  Constructor = 4,
  Field = 5,
  Variable = 6,
  Class = 7,
  Interface = 8,
  Module = 9,
  Property = 10,
  Unit = 11,
  Value = 12,
  Enum = 13,
  Keyword = 14,
  Snippet = 15,
  Color = 16,
  File = 17,
  Reference = 18,
  Folder = 19,
  EnumMember = 20,
  Constant = 21,
  Struct = 22,
  Event = 23,
  Operator = 24,
  TypeParameter = 25,
};
enum class SymbolKind {
  File = 1,
  Module = 2,
  Namespace = 3,
  Package = 4,
  Class = 5,
  Method = 6,
  Property = 7,
  Field = 8,
  Constructor = 9,
  Enum = 10,
  Interface = 11,
  Function = 12,
  Variable = 13,
  Constant = 14,
  String = 15,
  Number = 16,
  Boolean = 17,
  Array = 18,
  Object = 19,
  Key = 20,
  Null = 21,
  EnumMember = 22,
  Struct = 23,
  Event = 24,
  Operator = 25,
  TypeParameter = 26,
};
enum class MessageType {
  Error = 1,
  Warning = 2,
  Info = 3,
  Log = 4,
};
enum class FileChangeType {
  Created = 1,
  Changed = 2,
  Deleted = 3,
};
enum class CompletionTriggerKind {
  Invoked = 1,
  TriggerCharacter = 2,
  TriggerTriggerForIncompleteCompletions = 3,
};
enum class InsertTextFormat {
  Missing = 0,
  PlainText = 1,
  Snippet = 2,
};
enum class DocumentHighlightKind {
  Text = 1,
  Read = 2,
  Write = 3,
};
enum class TypeHierarchyDirection {
  Children = 0,
  Parents = 1,
  Both = 2,
};

// Enums with unspecified constants according to JSON RPC / LSP / Clangd

enum class OffsetEncoding {
  UnsupportedEncoding,
  UTF16,
  UTF8,
  UTF32,
};
enum class MarkupKind {
  PlainText,
  Markdown,
};
enum class ResourceOperationKind {
  Create,
  Rename,
  Delete,
};
enum class FailureHandlingKind {
  Abort,
  Transactional,
  Undo,
  TextOnlyTransactional
};
enum class FoldingRangeKind {
  Comment,
  Imports,
  Region,
};

#endif  // BATON_ENUMS_H
