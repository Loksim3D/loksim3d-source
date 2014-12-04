using System;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Effects;

namespace Loksim3D.ShaderEffectLibrary
{
    public class SimpleAlphaBlendingEffect : ShaderEffect
    {
        #region Constructors

        static SimpleAlphaBlendingEffect()
        {
            _pixelShader.UriSource = Global.MakePackUri("SimpleAlphaBlendingEffect.ps");
        }

        public SimpleAlphaBlendingEffect()
        {
            this.PixelShader = _pixelShader;

            // Update each DependencyProperty that's registered with a shader register.  This
            // is needed to ensure the shader gets sent the proper default value.
            UpdateShaderValue(Input1Property);
            UpdateShaderValue(Input2Property);
            UpdateShaderValue(AlphaValueProperty);
        }

        #endregion

        #region Dependency Properties

        public Brush Input1
        {
            get { return (Brush)GetValue(Input1Property); }
            set { SetValue(Input1Property, value); }
        }

        // Brush-valued properties turn into sampler-property in the shader.
        // This helper sets "ImplicitInput" as the default, meaning the default
        // sampler is whatever the rendering of the element it's being applied to is.
        public static readonly DependencyProperty Input1Property =
            ShaderEffect.RegisterPixelShaderSamplerProperty("Input1", typeof(SimpleAlphaBlendingEffect), 0);


        public Brush Input2
        {
            get { return (Brush)GetValue(Input2Property); }
            set { SetValue(Input2Property, value); }
        }

        // Brush-valued properties turn into sampler-property in the shader.
        // This helper sets "ImplicitInput" as the default, meaning the default
        // sampler is whatever the rendering of the element it's being applied to is.
        public static readonly DependencyProperty Input2Property =
            ShaderEffect.RegisterPixelShaderSamplerProperty("Input2", typeof(SimpleAlphaBlendingEffect), 1);


        public double AlphaValue
        {
            get { return (double)GetValue(AlphaValueProperty); }
            set { SetValue(AlphaValueProperty, value); }
        }

        // Scalar-valued properties turn into shader constants with the register
        // number sent into PixelShaderConstantCallback().
        public static readonly DependencyProperty AlphaValueProperty =
            DependencyProperty.Register("AlphaValue", typeof(double), typeof(SimpleAlphaBlendingEffect),
                    new UIPropertyMetadata(0.5, PixelShaderConstantCallback(0)));

        #endregion

        #region Member Data

        private static PixelShader _pixelShader = new PixelShader();

        #endregion

    }
}
