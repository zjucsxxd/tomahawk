/* === This file is part of Tomahawk Player - <http://tomahawk-player.org> ===
 *
 *   Copyright 2010-2011, Christian Muehlhaeuser <muesli@tomahawk-player.org>
 *
 *   Tomahawk is free software: you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation, either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   Tomahawk is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with Tomahawk. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FlexibleHeader.h"
#include "ui_PlaylistHeader.h"

#include <QLabel>
#include <QPixmap>
#include <QCheckBox>
#include <QPaintEvent>
#include <QPainter>

#include "playlist/FlexibleView.h"
#include "ViewManager.h"
#include "thirdparty/Qocoa/qsearchfield.h"
#include "utils/Closure.h"
#include "utils/TomahawkUtilsGui.h"
#include "utils/Logger.h"
#include "widgets/QueryLabel.h"
#include "Source.h"

using namespace Tomahawk;

QPixmap* FlexibleHeader::s_tiledHeader = 0;

FlexibleHeader::FlexibleHeader( FlexibleView* parent )
    : QWidget( parent )
    , m_parent( parent )
    , ui( new Ui::PlaylistHeader )
{
    ui->setupUi( this );

    TomahawkUtils::unmarginLayout( layout() );
    layout()->setContentsMargins( 8, 4, 8, 4 );

    QPalette pal = palette();
    pal.setColor( QPalette::Foreground, Qt::white );

    ui->captionLabel->setPalette( pal );
    ui->descLabel->setPalette( pal );

    QFont font = ui->captionLabel->font();
    font.setPointSize( TomahawkUtils::defaultFontSize() + 4 );
    font.setBold( true );
    ui->captionLabel->setFont( font );
    ui->captionLabel->setElideMode( Qt::ElideRight );

    font.setPointSize( TomahawkUtils::defaultFontSize() + 1 );
    font.setBold( false );
    ui->descLabel->setFont( font );

    ui->captionLabel->setMargin( 2 );
    ui->descLabel->setMargin( 1 );

    ui->radioNormal->setFocusPolicy( Qt::NoFocus );
    ui->radioDetailed->setFocusPolicy( Qt::NoFocus );
    ui->radioCloud->setFocusPolicy( Qt::NoFocus );

    QFile f( RESPATH "stylesheets/topbar-radiobuttons.css" );
    f.open( QFile::ReadOnly );
    QString css = QString::fromAscii( f.readAll() );
    f.close();

    ui->modeWidget->setStyleSheet( css );

    ui->radioNormal->setChecked( true );
    ui->filter->setPlaceholderText( tr( "Filter..." ) );

    pal = palette();
    pal.setColor( QPalette::Window, QColor( "#454e59" ) );

    setPalette( pal );
    setAutoFillBackground( true );

    setFixedHeight( 80 );

    if ( !s_tiledHeader )
        s_tiledHeader = new QPixmap( TomahawkUtils::createTiledPixmap( 2000, height(), QImage( RESPATH "images/playlist-header-tiled.png" ) ) );

    connect( &m_filterTimer, SIGNAL( timeout() ), SLOT( applyFilter() ) );
    connect( ui->filter, SIGNAL( textChanged( QString ) ), SLOT( onFilterEdited() ) );

    NewClosure( ui->radioNormal,   SIGNAL( clicked() ), const_cast< FlexibleView* >( parent ), SLOT( setCurrentMode( FlexibleViewMode ) ), FlexibleView::Flat )->setAutoDelete( false );
    NewClosure( ui->radioDetailed, SIGNAL( clicked() ), const_cast< FlexibleView* >( parent ), SLOT( setCurrentMode( FlexibleViewMode ) ), FlexibleView::Detailed )->setAutoDelete( false );
    NewClosure( ui->radioCloud,    SIGNAL( clicked() ), const_cast< FlexibleView* >( parent ), SLOT( setCurrentMode( FlexibleViewMode ) ), FlexibleView::Grid )->setAutoDelete( false );
}


FlexibleHeader::~FlexibleHeader()
{
    delete s_tiledHeader;
    s_tiledHeader = 0;
    delete ui;
}


void
FlexibleHeader::setCaption( const QString& s )
{
    ui->captionLabel->setText( s );
}


void
FlexibleHeader::setDescription( const QString& s )
{
    ui->descLabel->setText( s );
}


void
FlexibleHeader::setPixmap( const QPixmap& p )
{
    ui->imageLabel->setPixmap( p.scaledToHeight( ui->imageLabel->height(), Qt::SmoothTransformation ) );
}


void
FlexibleHeader::setFilter( const QString& filter )
{
    ui->filter->setText( filter );
}


void
FlexibleHeader::onFilterEdited()
{
    m_filter = ui->filter->text();

    m_filterTimer.stop();
    m_filterTimer.setInterval( 280 );
    m_filterTimer.setSingleShot( true );
    m_filterTimer.start();
}


void
FlexibleHeader::applyFilter()
{
    emit filterTextChanged( ui->filter->text() );
}


void
FlexibleHeader::changeEvent( QEvent* e )
{
    QWidget::changeEvent( e );
    switch ( e->type() )
    {
        case QEvent::LanguageChange:
//            ui->retranslateUi( this );
            break;

        default:
            break;
    }
}


void
FlexibleHeader::paintEvent( QPaintEvent*  )
{
    if ( !s_tiledHeader || s_tiledHeader->isNull() || width() > s_tiledHeader->width() )
    {
        delete s_tiledHeader;
        s_tiledHeader = new QPixmap( TomahawkUtils::createTiledPixmap( width(), height(), QImage( RESPATH "images/playlist-header-tiled.png" ) ) );
    }

    if ( !s_tiledHeader || s_tiledHeader->isNull() )
        return;

    QPainter p( this );

    // Truncate bg pixmap and paint into bg
    p.drawPixmap( rect(), *s_tiledHeader, rect() );
}
