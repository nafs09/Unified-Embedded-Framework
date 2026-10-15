using System.ComponentModel.Composition;
using Microsoft.VisualStudio.LanguageServer.Client;
using Microsoft.VisualStudio.Utilities;

namespace UEF.ProjectLanguage.VisualStudio
{
    // Keep the content-type provider public so Visual Studio's MEF catalog can
    // discover it in every supported IDE hive before matching .uef/.uefs files.
    public static class UETDProjectLanguageContentTypes
    {
        internal const string Name = "uetd-pl";

        [Export]
        [Name(Name)]
        [BaseDefinition(CodeRemoteContentDefinition.CodeRemoteContentTypeName)]
        internal static ContentTypeDefinition ContentType;

        [Export]
        [FileExtension(".uef")]
        [ContentType(Name)]
        internal static FileExtensionToContentTypeDefinition ProjectFileExtension;

        [Export]
        [FileExtension(".uefs")]
        [ContentType(Name)]
        internal static FileExtensionToContentTypeDefinition SystemFileExtension;
    }
}
