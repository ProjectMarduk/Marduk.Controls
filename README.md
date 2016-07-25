# Marduk.Controls
Yet another controls library for Universal Windows Apps.

# Nuget
[Marduk.Controls](https://www.nuget.org/packages/Marduk.Controls/)

```
PM> Install-Package Marduk.Controls
```

# Controls
## PhotowallView
A stand alone photo wall layout control with virtualizing capability.
![photowall](https://cloud.githubusercontent.com/assets/9367842/17103395/aeb31994-52b0-11e6-85a0-810188084535.gif)

## WaterfallFlowView
A stand alone waterfall flow layout controls with virtualizing capability.
![waterfall]
(https://cloud.githubusercontent.com/assets/9367842/17103352/8064d730-52b0-11e6-865d-bdd07396ed0c.gif)

# Features
## UI Virtualizing
Both of **PhotowallView** and **WaterfallFlowView** support UI virtualizing which enables the system to render only the visible items inside the viewport.

## Incremental Loading
Just make your ItemSource inherits form `Windows.UI.Xaml.Data.ISupportIncrementalLoading` then the control would do the rest.

## Item Tapped Event
Hooking up to the ItemTapped event with **PhotowallView** or **WaterfallFlowView** to get notified when an item was tapped.

## Easy Styling/ Comprehensive Visual States/ Multiselection
Easily styling the appearence of **PhotowallView** and **WaterfallFlowView** with the preset visual states such as "IsSelected" and "NotSelected" etc.

## High Performance for Random Insert, Remove and Change
Freely random insert, remove or change an item, even it's been virtualized. 

## High Performance for Resizing
You can use Marduk.Controls.OrientedVirtualizingPanel.Resizer to optimize resize behavior


# Quick Start
### 01. Create a WaterfallFlowView or PhotowallView inside a ScrollViewer, set some properties.

```XAML
<ScrollViewer>
    <controls:WaterfallFlowView xmlns:controls="using:Marduk.Controls" x:Name="Panel" 
    StackCount="3" DelayMeasure="True">
    </controls:WaterfallFlowView>
</ScrollViewer>
```

### 02. Binding your item source.

```XAML
<ScrollViewer>
    <controls:WaterfallFlowView xmlns:controls="using:Marduk.Controls" x:Name="Panel" 
    ItemSource="{Binding VM.Items}" StackCount="3" DelayMeasure="True">
    </controls:WaterfallFlowView>
</ScrollViewer>
```

### 03. Create ItemDataTemplate and ItemContainerStyle.

```XAML
<ScrollViewer>
    <controls:WaterfallFlowView xmlns:controls="using:Marduk.Controls" x:Name="Panel" 
    ItemSource="{Binding}" StackCount="3" DelayMeasure="True">
        <controls:WaterfallFlowView.ItemContainerStyle>
            <Style TargetType="ContentControl">
                <Setter Property="HorizontalAlignment" Value="Stretch"></Setter>
                <Setter Property="HorizontalContentAlignment" Value="Stretch"></Setter>
            </Style>
        </controls:WaterfallFlowView.ItemContainerStyle>
        <controls:WaterfallFlowView.ItemTemplate>
            <DataTemplate>
                <Border Height="{Binding Length}" Background="{Binding Brush}" HorizontalAlignment="Stretch">
                    <TextBlock FontSize="50" Text="{Binding Num}" HorizontalAlignment="Center" VerticalAlignment="Center"/>
                </Border>
            </DataTemplate>
        </controls:WaterfallFlowView.ItemTemplate>
    </controls:WaterfallFlowView>
</ScrollViewer>
```

### 04. Optimization for Resizing.
#### (1) Create a resizer class which implements Marduk.Controls.IItemResizer.

```CSharp
public class MyItemResizer : IItemResizer
{
    public Size Resize(object item, Size oldSize, Size availableSize)
    {
        return new Size(availableSize.Width, oldSize.Height);
    }
}
```
#### (2) Assign the resize to the WaterfallFlowView.

```XAML
<ScrollViewer>
    <controls:WaterfallFlowView xmlns:controls="using:Marduk.Controls" x:Name="Panel" ItemSource="{Binding}" StackCount="3" DelayMeasure="True">
        <controls:WaterfallFlowView.Resizer>
            <local:MyItemResizer/>
        </controls:WaterfallFlowView.Resizer>
        <controls:WaterfallFlowView.ItemContainerStyle>
            <Style TargetType="ContentControl">
                <Setter Property="HorizontalAlignment" Value="Stretch"></Setter>
                <Setter Property="HorizontalContentAlignment" Value="Stretch"></Setter>
            </Style>
        </controls:WaterfallFlowView.ItemContainerStyle>
        <controls:WaterfallFlowView.ItemTemplate>
            <DataTemplate>
                <Border Height="{Binding Length}" Background="{Binding Brush}" HorizontalAlignment="Stretch">
                    <TextBlock FontSize="50" Text="{Binding Num}" HorizontalAlignment="Center" VerticalAlignment="Center"/>
                </Border>
            </DataTemplate>
        </controls:WaterfallFlowView.ItemTemplate>
    </controls:WaterfallFlowView>
</ScrollViewer>
```
