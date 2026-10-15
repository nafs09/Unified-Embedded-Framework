using System.ComponentModel.Composition;
using System.Windows.Media;
using Microsoft.VisualStudio.Text.Classification;
using Microsoft.VisualStudio.Text.Formatting;
using Microsoft.VisualStudio.Utilities;

namespace UEF.ProjectLanguage.VisualStudio
{
    internal static class UEFProjectLanguageClassificationTypes
    {
        internal const string ProjectKeywordName = "UEFPL.ProjectKeyword";
        internal const string TypeKeywordName = "UEFPL.TypeKeyword";
        internal const string HardwareKeywordName = "UEFPL.HardwareKeyword";
        internal const string AlgorithmKeywordName = "UEFPL.AlgorithmKeyword";
        internal const string RuntimeKeywordName = "UEFPL.RuntimeKeyword";
        internal const string BindingKeywordName = "UEFPL.BindingKeyword";
        internal const string DeclarationNameName = "UEFPL.DeclarationName";
        internal const string TypeDimensionName = "UEFPL.TypeDimension";
        internal const string ProjectNameName = "UEFPL.ProjectName";
        internal const string HardwareNameName = "UEFPL.HardwareName";
        internal const string AlgorithmNameName = "UEFPL.AlgorithmName";
        internal const string RuntimeNameName = "UEFPL.RuntimeName";
        internal const string TypeNameName = "UEFPL.TypeName";
        internal const string RecordFieldName = "UEFPL.RecordField";
        internal const string EnumMemberName = "UEFPL.EnumMember";
        internal const string NumberName = "UEFPL.Number";

        [Export(typeof(ClassificationTypeDefinition)), Name(ProjectKeywordName)]
        internal static ClassificationTypeDefinition ProjectKeyword;

        [Export(typeof(ClassificationTypeDefinition)), Name(TypeKeywordName)]
        internal static ClassificationTypeDefinition TypeKeyword;

        [Export(typeof(ClassificationTypeDefinition)), Name(HardwareKeywordName)]
        internal static ClassificationTypeDefinition HardwareKeyword;

        [Export(typeof(ClassificationTypeDefinition)), Name(AlgorithmKeywordName)]
        internal static ClassificationTypeDefinition AlgorithmKeyword;

        [Export(typeof(ClassificationTypeDefinition)), Name(RuntimeKeywordName)]
        internal static ClassificationTypeDefinition RuntimeKeyword;

        [Export(typeof(ClassificationTypeDefinition)), Name(BindingKeywordName)]
        internal static ClassificationTypeDefinition BindingKeyword;

        [Export(typeof(ClassificationTypeDefinition)), Name(DeclarationNameName)]
        internal static ClassificationTypeDefinition DeclarationName;

        [Export(typeof(ClassificationTypeDefinition)), Name(TypeDimensionName)]
        internal static ClassificationTypeDefinition TypeDimension;

        [Export(typeof(ClassificationTypeDefinition)), Name(ProjectNameName)]
        internal static ClassificationTypeDefinition ProjectName;

        [Export(typeof(ClassificationTypeDefinition)), Name(HardwareNameName)]
        internal static ClassificationTypeDefinition HardwareName;

        [Export(typeof(ClassificationTypeDefinition)), Name(AlgorithmNameName)]
        internal static ClassificationTypeDefinition AlgorithmName;

        [Export(typeof(ClassificationTypeDefinition)), Name(RuntimeNameName)]
        internal static ClassificationTypeDefinition RuntimeName;

        [Export(typeof(ClassificationTypeDefinition)), Name(TypeNameName)]
        internal static ClassificationTypeDefinition TypeName;

        [Export(typeof(ClassificationTypeDefinition)), Name(RecordFieldName)]
        internal static ClassificationTypeDefinition RecordField;

        [Export(typeof(ClassificationTypeDefinition)), Name(EnumMemberName)]
        internal static ClassificationTypeDefinition EnumMember;

        [Export(typeof(ClassificationTypeDefinition)), Name(NumberName)]
        internal static ClassificationTypeDefinition Number;
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.ProjectKeywordName)]
    [Name("UEF-PL project declarations")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFProjectKeywordFormat : ClassificationFormatDefinition
    {
        public UEFProjectKeywordFormat()
        {
            DisplayName = "UEF-PL Project Declaration";
            ForegroundColor = Color.FromRgb(79, 193, 255);
            IsBold = true;
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.TypeKeywordName)]
    [Name("UEF-PL type declarations")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFTypeKeywordFormat : ClassificationFormatDefinition
    {
        public UEFTypeKeywordFormat()
        {
            DisplayName = "UEF-PL Type Declaration";
            ForegroundColor = Color.FromRgb(78, 201, 176);
            IsBold = true;
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.HardwareKeywordName)]
    [Name("UEF-PL hardware declarations")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFHardwareKeywordFormat : ClassificationFormatDefinition
    {
        public UEFHardwareKeywordFormat()
        {
            DisplayName = "UEF-PL Hardware Declaration";
            ForegroundColor = Color.FromRgb(240, 140, 70);
            IsBold = true;
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.AlgorithmKeywordName)]
    [Name("UEF-PL algorithm declarations")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFAlgorithmKeywordFormat : ClassificationFormatDefinition
    {
        public UEFAlgorithmKeywordFormat()
        {
            DisplayName = "UEF-PL Algorithm Declaration";
            ForegroundColor = Color.FromRgb(197, 134, 192);
            IsBold = true;
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.RuntimeKeywordName)]
    [Name("UEF-PL runtime declarations")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFRuntimeKeywordFormat : ClassificationFormatDefinition
    {
        public UEFRuntimeKeywordFormat()
        {
            DisplayName = "UEF-PL Runtime Declaration";
            ForegroundColor = Color.FromRgb(111, 175, 80);
            IsBold = true;
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.BindingKeywordName)]
    [Name("UEF-PL data binding declarations")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFBindingKeywordFormat : ClassificationFormatDefinition
    {
        public UEFBindingKeywordFormat()
        {
            DisplayName = "UEF-PL Binding Keyword";
            ForegroundColor = Color.FromRgb(86, 156, 214);
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.DeclarationNameName)]
    [Name("UEF-PL declaration names")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFDeclarationNameFormat : ClassificationFormatDefinition
    {
        public UEFDeclarationNameFormat()
        {
            DisplayName = "UEF-PL Declaration Name";
            ForegroundColor = Color.FromRgb(156, 220, 254);
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.TypeDimensionName)]
    [Name("UEF-PL type dimensions")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFTypeDimensionFormat : ClassificationFormatDefinition
    {
        public UEFTypeDimensionFormat()
        {
            DisplayName = "UEF-PL Type Dimension";
            ForegroundColor = Color.FromRgb(212, 212, 212);
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.ProjectNameName)]
    [Name("UEF-PL project names")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFProjectNameFormat : ClassificationFormatDefinition
    {
        public UEFProjectNameFormat()
        {
            DisplayName = "UEF-PL Project Name";
            ForegroundColor = Color.FromRgb(138, 215, 255);
            IsBold = true;
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.HardwareNameName)]
    [Name("UEF-PL hardware names")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFHardwareNameFormat : ClassificationFormatDefinition
    {
        public UEFHardwareNameFormat()
        {
            DisplayName = "UEF-PL Hardware Name";
            ForegroundColor = Color.FromRgb(255, 179, 107);
            IsBold = true;
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.AlgorithmNameName)]
    [Name("UEF-PL algorithm names")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFAlgorithmNameFormat : ClassificationFormatDefinition
    {
        public UEFAlgorithmNameFormat()
        {
            DisplayName = "UEF-PL Algorithm Name";
            ForegroundColor = Color.FromRgb(221, 160, 221);
            IsBold = true;
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.RuntimeNameName)]
    [Name("UEF-PL runtime names")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFRuntimeNameFormat : ClassificationFormatDefinition
    {
        public UEFRuntimeNameFormat()
        {
            DisplayName = "UEF-PL runtime name";
            ForegroundColor = Color.FromRgb(180, 226, 139);
            IsBold = true;
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.TypeNameName)]
    [Name("UEF-PL declared type names")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFTypeNameFormat : ClassificationFormatDefinition
    {
        public UEFTypeNameFormat()
        {
            DisplayName = "UEF-PL Type Name";
            ForegroundColor = Color.FromRgb(118, 215, 197);
            IsBold = true;
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.RecordFieldName)]
    [Name("UEF-PL record fields")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFRecordFieldFormat : ClassificationFormatDefinition
    {
        public UEFRecordFieldFormat()
        {
            DisplayName = "UEF-PL Record Field";
            ForegroundColor = Color.FromRgb(156, 220, 254);
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.EnumMemberName)]
    [Name("UEF-PL enum members")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFEnumMemberFormat : ClassificationFormatDefinition
    {
        public UEFEnumMemberFormat()
        {
            DisplayName = "UEF-PL Enum Member";
            ForegroundColor = Color.FromRgb(197, 134, 192);
        }
    }

    [Export(typeof(EditorFormatDefinition))]
    [ClassificationType(ClassificationTypeNames = UEFProjectLanguageClassificationTypes.NumberName)]
    [Name("UEF-PL numeric literals")]
    [UserVisible(true)]
    [Order(After = Priority.Default)]
    internal sealed class UEFNumberFormat : ClassificationFormatDefinition
    {
        public UEFNumberFormat()
        {
            DisplayName = "UEF-PL Number";
            ForegroundColor = Color.FromRgb(212, 212, 212);
        }
    }
}
